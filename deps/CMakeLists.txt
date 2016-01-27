
set(HARFBUZZ_BUILD_ICU ON CACHE BOOL "Enable building of ICU")
set(HARFBUZZ_BUILD_UCDN ON CACHE BOOL "Enable building of UCDN")

add_subdirectory(hb-icu-ft)
