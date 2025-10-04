#include <math.h>
#include <stdbool.h>

// Vom System bereitgestellt:
float readTu(void);
void  setHeater(int level);

// Vorgaben (können global/extern kommen)
float Ts = 50.0f; // Sollwert in °C

// ---------------- Parameter (gute Startwerte) ----------------
#define DT_SEC              (0.1f)     // 100 ms

// Tu-Filter (einfacher IIR: y += a*(x - y))
#define TU_FILTER_TAU_SEC   (0.8f)     // Sensor glätten, aber flott bleiben

// PI-Regler (auf Tu) — auf PWM-Range 0..128 abgestimmt
// Herleitung: aus vorherigem Modell KP~30 (0..255) -> hier ~halb so groß
#define KP                  (15.0f)    // proportionaler Gain
// Verhältnis KI/KP ~ 0.00183 1/s -> KI ~ 0.0275 1/s bei KP=15
#define KI                  (0.0275f)  // Integralgain [1/s]

// Integrator-Grenzen (als PWM-Äquivalent)
#define I_MIN               (0.0f)
#define I_MAX               (128.0f)

// Ausgangsbegrenzung
#define PWM_MIN             (0)
#define PWM_MAX             (128)

// Slew-Rate-Begrenzung (max. Änderung pro 100 ms)
#define PWM_SLEW_PER_TICK   (40)       // zügig, bei Bedarf kleiner wählen

// Totzone um Ts (verhindert Sägen)
#define TU_BAND             (0.2f)     // in K

// Aufheizphase: mit Boost bis kurz vor Ts
#define WARMUP_MARGIN       (1.0f)     // Umschalten auf PI bei Ts - 1 K
#define BOOST_EXTRA         (32)       // zusätzl. PWM über dem Halt-Level, max 128

// Schätzung für initialen Halt-PWM (nur für schnellere Annäherung)
// grob: PWM ≈ G * (Ts - Tu0), mit G ~ 3.8 Counts/K (für Range 0..128)
#define HOLD_GAIN_EST       (3.8f)

// -------------------------------------------------------------

typedef enum { PHASE_WARMUP = 0, PHASE_HOLD = 1 } phase_t;

static struct {
    // Zustände
    float Tu_f;          // gefiltertes Tu
    float I;             // Integrator (als PWM-Äquivalent 0..128)
    int   pwm;           // letzter ausgegebener PWM-Wert
    phase_t phase;       // Warmup/Hold
    float Tu0;           // Start-Umgebung für Schätzung
    bool  initialized;   // Filter init?
    // Merker für „bumpless transfer“
    bool  hold_initialized;
    int   pwm_hold_est;  // initiale Schätzung des Halt-PWM
} ctl = {0};

// Hilfen
static inline float clampf(float x, float lo, float hi){
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}
static inline int clampi(int x, int lo, int hi){
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

void controlHeater(void)
{
    // 1) Messung & Initialisierung
    float Tu = readTu();

    if (!ctl.initialized) {
        ctl.Tu_f  = Tu;
        ctl.Tu0   = Tu;
        ctl.I     = 0.0f;
        ctl.pwm   = 0;
        ctl.phase = PHASE_WARMUP;
        ctl.hold_initialized = false;

        // Grobe Halt-Schätzung aus Startdifferenz
        float dT = Ts - Tu;
        int pwm_est = (int)lroundf(HOLD_GAIN_EST * dT);
        ctl.pwm_hold_est = clampi(pwm_est, PWM_MIN, PWM_MAX);

        ctl.initialized = true;
    }

    // 2) Tu filtern
    const float a_tu = DT_SEC / (TU_FILTER_TAU_SEC + DT_SEC);
    ctl.Tu_f += a_tu * (Tu - ctl.Tu_f);

    // 3) Sofort AUS, wenn Ts <= 0
    if (Ts <= 0.0f) {
        ctl.I = 0.0f;
        ctl.pwm = 0;
        ctl.phase = PHASE_WARMUP;
        ctl.hold_initialized = false;
        setHeater(0);
        return;
    }

    // 4) Phasenlogik
    int u_cmd = 0;

    if (ctl.phase == PHASE_WARMUP) {
        // Warmup: so schnell wie sinnvoll nach oben
        int u_boost = ctl.pwm_hold_est + BOOST_EXTRA;
        u_cmd = clampi(u_boost, PWM_MIN, PWM_MAX);

        // Umschalten auf PI kurz vor Soll
        if (ctl.Tu_f >= (Ts - WARMUP_MARGIN)) {
            ctl.phase = PHASE_HOLD;
        }
    }

    if (ctl.phase == PHASE_HOLD) {
        // 4a) Integrator beim ersten Eintritt „bumpless“ an das aktuelle Niveau angleichen
        if (!ctl.hold_initialized) {
            // Starte Integrator nahe dem aktuellen (oder geschätzten) Halt-PWM
            float I0 = (float)clampi(ctl.pwm, PWM_MIN, PWM_MAX);
            if (I0 < 1.0f) I0 = (float)ctl.pwm_hold_est; // Falls wir mit 0 ankamen
            ctl.I = clampf(I0, I_MIN, I_MAX);
            ctl.hold_initialized = true;
        }

        // 4b) PI auf Tu
        float e = Ts - ctl.Tu_f;
        if (fabsf(e) < TU_BAND) e = 0.0f;

        float P = KP * e;
        float I_candidate = ctl.I + (KI * e * DT_SEC);
        I_candidate = clampf(I_candidate, I_MIN, I_MAX);

        float u_pid = P + I_candidate;

        // Begrenzen und Antiwindup (Top-Limit)
        int u_unsat = (int)lroundf(u_pid);
        int u_sat   = clampi(u_unsat, PWM_MIN, PWM_MAX);

        bool limited_top = (u_sat < u_unsat) && (e > 0.0f);
        if (!limited_top) {
            ctl.I = I_candidate;
        }

        u_cmd = u_sat;

        // Optional: Wenn wir wieder deutlich unter Ts fallen (z. B. Tür auf),
        // zurück in Warmup für schnelles Nachheizen:
        if (ctl.Tu_f < Ts - (WARMUP_MARGIN + 0.5f)) {
            ctl.phase = PHASE_WARMUP;
            ctl.hold_initialized = false;
            // Neue Halt-Schätzung aus aktueller Umgebung
            float dT = Ts - ctl.Tu_f;
            int pwm_est = (int)lroundf(HOLD_GAIN_EST * dT);
            ctl.pwm_hold_est = clampi(pwm_est, PWM_MIN, PWM_MAX);
            // u_cmd wird unten noch geslewt & gesetzt
        }
    }

    // 5) Slew-Rate-Begrenzung
    int max_up   = ctl.pwm + PWM_SLEW_PER_TICK;
    int max_down = ctl.pwm - PWM_SLEW_PER_TICK;
    int u_slewed = u_cmd;
    if (u_slewed > max_up)   u_slewed = max_up;
    if (u_slewed < max_down) u_slewed = max_down;

    // 6) Ausgeben
    ctl.pwm = clampi(u_slewed, PWM_MIN, PWM_MAX);
    setHeater(ctl.pwm);
}
