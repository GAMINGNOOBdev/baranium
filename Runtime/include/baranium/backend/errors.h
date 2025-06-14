/**
 * @note THIS IS NOT INTENDED FOR USE BY THE USER OF THE RUNTIME!
 *       This header is intended to be used internally by the runtime
 *       and therefore, functions defined in this header cannot be used
 *       by the user.
 */
#ifndef __BARANIUM__BACKEND__ERRORS_H_
#define __BARANIUM__BACKEND__ERRORS_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define BARANIUM_ERROR_VAR_NOT_FOUND        0x01
#define BARANIUM_ERROR_VAR_WRONG_SIZE       0x02
#define BARANIUM_ERROR_VAR_INVALID_TYPE     0x03
#define BARANIUM_ERROR_DIV_BY_ZERO          0x04
#define BARANIUM_ERROR_OUT_OF_MEMORY        0x05

#define BARANIUM_ERROR_TO_STRING(error) \
    (error == BARANIUM_ERROR_VAR_NOT_FOUND) ? "VAR_NOT_FOUND" : \
    (error == BARANIUM_ERROR_VAR_WRONG_SIZE) ? "VAR_WRONG_SIZE" : \
    (error == BARANIUM_ERROR_VAR_INVALID_TYPE) ? "VAR_INVALID_TYPE" : \
    (error == BARANIUM_ERROR_DIV_BY_ZERO) ? "DIV_BY_ZERO" : \
    (error == BARANIUM_ERROR_OUT_OF_MEMORY) ? "OUT_OF_MEMORY" : "UNKNOWN"

#ifdef __cplusplus
}
#endif

#endif
