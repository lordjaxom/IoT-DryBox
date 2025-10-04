#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "Heating.h"

// ---------------- Parameter für 1 Hz ----------------
#define DT_SEC                 (1.0f)    // 1 Aufruf pro Sekunde
#define TU_FILTER_TAU_SEC      (1.2f)    // etwas größer bei 1 Hz

// PI-Gains (auf PWM 0..128 abgestimmt, 1/s)
#define KP                     (10.0f)   // etwas kleiner als bei 10 Hz
#define KI_BASE                (0.0275f) // weiterhin pro Sekunde
#define KI_BLEED_FACTOR        (3.5f)    // schnelleres Entladen bei e < 0

#define I_MIN                  (0.0f)
#define I_MAX                  (128.0f)

#define PWM_MIN                (0)
#define PWM_MAX                (160)
#define PWM_SLEW_PER_TICK      (16)      // 16 Counts pro Sekunde

#define TU_BAND                (0.2f)    // Totzone

// Zweistufiges Warmup (A=Vollgas, B=Taper)
#define WARMUP_A_FULL_MARGIN   (4.0f)    // bis Ts-4 K = 128
#define WARMUP_B_TAPER_MARGIN  (1.2f)    // bis Ts-1.2 K Taper -> hold

// Haltleistung-Schätzer (Range 0..128)
#define HOLD_GAIN_EST          (3.8f)

typedef enum { PHASE_WARMUP = 0, PHASE_HOLD = 1 } phase_t;

static struct {
    float Tu_f;          // gefiltertes Tu
    float Tu_f_prev;     // für dTu/dt
    float I;             // Integrator
    int   pwm;           // aktueller PWM
    phase_t phase;       // Phase
    bool  initialized;
    bool  hold_initialized;
    float Tu0;           // Start-Umgebung
    // Reporting
    float last_e, last_P, last_I, last_dTudt;
} ctl = {0};

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
static inline int estimate_hold_pwm(float Ts_local, float Tu0_local){
    float dT0 = Ts_local - Tu0_local;
    int est = (int)lroundf(HOLD_GAIN_EST * dT0);
    return clampi(est, PWM_MIN, PWM_MAX);
}

void controlHeater(float Ts)
{
    float Tu = readTemperature();

    // Init
    if (!ctl.initialized) {
        ctl.Tu_f  = Tu;
        ctl.Tu_f_prev = Tu;
        ctl.Tu0   = Tu;
        ctl.I     = 0.0f;
        ctl.pwm   = 0;
        ctl.phase = PHASE_WARMUP;
        ctl.hold_initialized = false;
        ctl.initialized = true;
        ctl.last_e = ctl.last_P = ctl.last_I = ctl.last_dTudt = 0.0f;
    }

    // Filter
    const float a_tu = DT_SEC / (TU_FILTER_TAU_SEC + DT_SEC);
    ctl.Tu_f += a_tu * (Tu - ctl.Tu_f);

    // dTu/dt (gefiltert, K/s)
    float dTudt = (ctl.Tu_f - ctl.Tu_f_prev) / DT_SEC;
    ctl.Tu_f_prev = ctl.Tu_f;

    // AUS bei Ts<=0
    if (Ts <= 0.0f) {
        ctl.I = 0.0f;
        ctl.pwm = 0;
        ctl.phase = PHASE_WARMUP;
        ctl.hold_initialized = false;
        setHeater(0);
        // Reporting
        char buf[192];
        snprintf(buf, sizeof(buf),
                 "Tu=%.2f Ts=%.1f e=%.2f dTudt=%.2f P=%.2f I=%.2f PWM=%d phase=%s",
                 ctl.Tu_f, Ts, Ts - ctl.Tu_f, dTudt,
                 0.0f, ctl.I, ctl.pwm, "Warmup");
        reportValues(buf);
        return;
    }

    // Fehler
    float e_raw = Ts - ctl.Tu_f;
    ctl.last_e = e_raw;

    int u_cmd = 0;

    // ---------------- Warmup ----------------
    if (ctl.phase == PHASE_WARMUP) {
        int hold_est = estimate_hold_pwm(Ts, ctl.Tu0);

        if (ctl.Tu_f < (Ts - WARMUP_A_FULL_MARGIN)) {
            // Stage A: Vollgas
            u_cmd = PWM_MAX;
        } else if (ctl.Tu_f < (Ts - WARMUP_B_TAPER_MARGIN)) {
            // Stage B: linear 128 -> hold_est
            float span = (WARMUP_A_FULL_MARGIN - WARMUP_B_TAPER_MARGIN); // >0
            float x = (Ts - WARMUP_B_TAPER_MARGIN - ctl.Tu_f) / span;    // 1..0
            x = clampf(x, 0.0f, 1.0f);
            float u_f = hold_est + x * (PWM_MAX - hold_est);
            u_cmd = clampi((int)lroundf(u_f), PWM_MIN, PWM_MAX);
        } else {
            // Übergabe an HOLD
            ctl.phase = PHASE_HOLD;
            ctl.hold_initialized = false;
        }

        ctl.last_P = 0.0f;
        ctl.last_I = ctl.I;
    }

    // ---------------- Hold (PI) ----------------
    if (ctl.phase == PHASE_HOLD) {
        // Integrator „bumpless“ an Haltleistung koppeln
        if (!ctl.hold_initialized) {
            ctl.I = (float)estimate_hold_pwm(Ts, ctl.Tu0);
            ctl.I = clampf(ctl.I, I_MIN, I_MAX);
            ctl.hold_initialized = true;
        }

        float e = e_raw;
        if (fabsf(e) < TU_BAND) e = 0.0f;

        float P = KP * e;
        float KI = (e < 0.0f) ? (KI_BASE * KI_BLEED_FACTOR) : KI_BASE;

        float I_candidate = ctl.I + (KI * e * DT_SEC);
        I_candidate = clampf(I_candidate, I_MIN, I_MAX);

        float u_pid = P + I_candidate;
        int u_unsat = (int)lroundf(u_pid);
        int u_sat   = clampi(u_unsat, PWM_MIN, PWM_MAX);

        // Antiwindup: oben nicht weiter integrieren
        bool limited_top = (u_sat < u_unsat) && (e > 0.0f);
        if (!limited_top) {
            ctl.I = I_candidate;
        }

        // bei deutlichem Abfall zurück in Warmup
        if (ctl.Tu_f < Ts - (WARMUP_A_FULL_MARGIN + 0.5f)) {
            ctl.phase = PHASE_WARMUP;
            ctl.hold_initialized = false;
        }

        // Slew pro Sekunde
        int u_out = u_sat;
        int max_up   = ctl.pwm + PWM_SLEW_PER_TICK;
        int max_down = ctl.pwm - PWM_SLEW_PER_TICK;
        if (u_out > max_up)   u_out = max_up;
        if (u_out < max_down) u_out = max_down;

        u_cmd = u_out;

        ctl.last_P = P;
        ctl.last_I = ctl.I;
    }

    // Ausgabe
    ctl.pwm = clampi(u_cmd, PWM_MIN, PWM_MAX);
    setHeater(ctl.pwm);

    // Reporting (jetzt bei jedem Call = 1 Hz)
    ctl.last_dTudt = dTudt;
    char buf[192];
    snprintf(buf, sizeof(buf),
             "Tu=%.2f Ts=%.1f e=%.2f dTudt=%.2f P=%.2f I=%.2f PWM=%d phase=%s hold_est=%d",
             ctl.Tu_f, Ts, ctl.last_e, ctl.last_dTudt,
             ctl.last_P, ctl.last_I, ctl.pwm,
             (ctl.phase==PHASE_WARMUP ? "Warmup" : "Hold"),
             estimate_hold_pwm(Ts, ctl.Tu0));
    reportValues(buf);
}
