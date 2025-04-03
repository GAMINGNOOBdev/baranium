#ifndef __BARANIUM__VERSION_H_
#define __BARANIUM__VERSION_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define BARANIUM_VERSION_YEAR        __YEAR__
#define BARANIUM_VERSION_MONTH       __MONTH__
#define BARANIUM_VERSION_DATE        __DAY__
#define BARANIUM_VERSION_PHASE       "release"

#define BARANIUM_VERSION_CREATE(year, month, day) ((year << 16) | (month << 8) | day)

#define BARANIUM_VERSION_FIRST BARANIUM_VERSION_CREATE(2024, 2, 10)
#define BARANIUM_VERSION_FIRST_RELEASE BARANIUM_VERSION_CREATE(2025,1,6)
#define BARANIUM_VERSION_SECOND_RELEASE BARANIUM_VERSION_CREATE(2025,4,2)
#define BARANIUM_VERSION_CURRENT BARANIUM_VERSION_CREATE(BARANIUM_VERSION_YEAR, BARANIUM_VERSION_MONTH, BARANIUM_VERSION_DATE)

#ifdef __cplusplus
}
#endif

#endif
