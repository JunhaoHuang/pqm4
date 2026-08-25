/* Cortex-M4 m4-fxp deliberately has no diagnostic paths in secret processing. */
#ifndef FALCON_FXP_DEBUG_H__
#define FALCON_FXP_DEBUG_H__
#define print_debug(x) do { } while (0)
#define print_fpapprox_buf(x, y, z) do { } while (0)
#define print_fp_buf(x, y, z) do { } while (0)
#define print_fp(x) do { } while (0)
#define print_buf32(x, y, z) do { } while (0)
#define print_buf16(x, y, z) do { } while (0)
#define print_buf8(x, y, z) do { } while (0)
#define print_poly(x, y) do { } while (0)
#define print_marker(x) do { } while (0)
#define set_debug(x) do { } while (0)
#define approx(x) 0.0
#endif
