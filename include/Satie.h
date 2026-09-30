#ifndef SATIE_C_H
#define SATIE_C_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SatieCStatus {
    SATIE_C_SAT = 1,
    SATIE_C_UNSAT = 0,
    SATIE_C_UNKNOWN = -1
} SatieCStatus;

#ifdef __cplusplus
}
#endif

#endif /* SATIE_C_H */
