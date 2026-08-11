# Conic Gradient — User Guide

The conic gradient feature paints a color sweep around a center point.
This document is for application developers.

- Public API: `inc/vg_lite.h`
- Implementation: `VGLite/vg_lite_path.c`
- Pre-generated angle LUT: `VGLite/vg_lite_atan.h`

## rendering
![demo](watchface.apng)

The demo is available at: https://github.com/VeriSilicon/VGLite_Tests/tree/REL/4.0.135_conic/VSI_CTS/samples/conic

## 1. Feature availability

Conic gradient is compiled in only when the chip's
`gcFEATURE_VG_CONIC_GRADIENT` option is `1`.

## 2. Data model

```c
vg_lite_conic_gradient_parameter_t param = {
    .cx          = 240.0f,   /* center x, path space */
    .cy          = 240.0f,   /* center y, path space */
    .radius      = 220.0f,   /* wheel radius, path space */
    .start_angle = 0.0f,     /* degrees, see section 4 */
};

vg_lite_color_ramp_t stops[N] = {
    { .stop = <t in [0,1]>, .red, .green, .blue, .alpha },
    ...
};

vg_lite_conic_gradient_t grad;
memset(&grad, 0, sizeof(grad));           /* must zero */
```

Requirements on `stops`:

- `2 <= N <= 16`
- All `stop` values in `[0, 1]`
- Non-decreasing: `stop[i].stop <= stop[i+1].stop`
- `stop[0].stop < stop[N-1].stop` (span must be non-zero)

## 3. Lifecycle

```c
/* 1. Configure */
CHECK_ERROR(vg_lite_conic_grad_set(
    &grad, N, stops, param,
    VG_LITE_GRADIENT_SPREAD_PAD,  /* see section 6 */
    /* pre_multiplied = */ 0));

/* 2. Bake the angle texture (only re-runs if ramp/geometry is dirty) */
CHECK_ERROR(vg_lite_conic_grad_update(&grad));

/* 3. Draw as many times as you like */
vg_lite_matrix_t identity;
vg_lite_identity(&identity);
CHECK_ERROR(vg_lite_conic_grad_draw(
    fb, &path, VG_LITE_FILL_EVEN_ODD, &identity, &grad,
    VG_LITE_BLEND_NONE, VG_LITE_FILTER_POINT));

/* 4. Release the internal texture when done */
vg_lite_conic_grad_clear(&grad);
```

After changing the ramp or geometry, call `vg_lite_conic_grad_set`
again followed by `vg_lite_conic_grad_update`; the update will
re-bake automatically.  `vg_lite_conic_grad_clear` releases the
texture and zeroes the struct.

## 4. Angular convention

- With `start_angle = 0`, **t = 0 aligns with the +x axis (3 o'clock)**.
- **Positive `start_angle` rotates clockwise** (Y-down screen
  coordinates).

## 5. Pre-multiplied colors

| `pre_multiplied` | Meaning                                                     |
|------------------|-------------------------------------------------------------|
| `0`              | Straight color, no alpha multiplication.                    |
| `1`              | Driver multiplies RGB by alpha when it bakes the texture.   |

## 6. Spread mode

Passed as the `spread_mode` argument to `vg_lite_conic_grad_set`.
`t` here is the normalized angular position (0..1 sweeps once around
the wheel starting at `start_angle`).

| Mode                              | `t < stop[0]`               | `t > stop[N-1]`             |
|-----------------------------------|-----------------------------|-----------------------------|
| `VG_LITE_GRADIENT_SPREAD_PAD`     | color of `stop[0]`          | color of `stop[N-1]`        |
| `VG_LITE_GRADIENT_SPREAD_REPEAT`  | wraps: `t <- t mod 1`       | wraps: `t <- t mod 1`       |
| `VG_LITE_GRADIENT_SPREAD_REFLECT` | mirrors: bounces at 0 and 1 | mirrors: bounces at 0 and 1 |
| `VG_LITE_GRADIENT_SPREAD_FILL`    | Same as PAD (current impl)  | Same as PAD (current impl)  |

Notes:

- REPEAT and REFLECT operate on the segment `[stop[0], stop[N-1]]`,
  not on `[0, 1]`.  A ramp with `stop[0]=0.2` and `stop[N-1]=0.7`
  under REPEAT tiles with period `0.5` in `t`.

## 7. Compile-time knobs (`VGLite/vg_lite_atan.h`)

```c
#define CONIC_ATAN_TEX_SIZE       256   /* 64 | 128 | 256 */
#define CONIC_ATAN_QUARTER_LUT    0     /* 0 = full NxN, 1 = quarter ref+qmap */
#define CONIC_ATAN_RUNTIME_CONFIG 0     /* 0 = compile-time only, 1 = runtime override */
```

Trade-offs:

- Larger `TEX_SIZE` -> sharper angles, more Flash (idx table 4 / 16 /
  64 KB for size 64 / 128 / 256).
- `QUARTER_LUT=1` uses a quarter-symmetric decomposition to cut ROM
  ~75% at the cost of a small extra CPU step per pixel during bake.
- `RUNTIME_CONFIG=1` compiles in all six tables (3 sizes x 2 layouts,
  ~110 KB total ROM) and lets the app switch at runtime through the
  API below.

## 8. Runtime override (optional)

Available only when built with `CONIC_ATAN_RUNTIME_CONFIG=1`.

```c
vg_lite_error_t vg_lite_conic_grad_set_tex_config(
    vg_lite_conic_gradient_t* grad,
    vg_lite_uint32_t          tex_size,        /* 0 | 64 | 128 | 256 */
    vg_lite_uint8_t           use_quarter_lut  /* 0 = full, 1 = quarter */);
```

- `tex_size = 0` means "use the compile-time default".
- Any other value not in `{64, 128, 256}` is rejected with
  `VG_LITE_INVALID_ARGUMENT`.
- Call **before** `vg_lite_conic_grad_update`; the override marks the
  gradient dirty so the next update re-bakes at the new size.
- On builds with `CONIC_ATAN_RUNTIME_CONFIG=0` the function returns
  `VG_LITE_NOT_SUPPORT` regardless of arguments.

Typical use:

```c
#ifdef CONIC_ATAN_RUNTIME_CONFIG
CHECK_ERROR(vg_lite_conic_grad_set_tex_config(&grad, 128, 0));
#endif
CHECK_ERROR(vg_lite_conic_grad_update(&grad));
```

## 9. Minimal example

Red-to-blue sweep with PAD outside `[0.2, 0.7]`:

```c
vg_lite_conic_gradient_t grad;
vg_lite_conic_gradient_parameter_t param = {240, 240, 220, 0.0f};
vg_lite_color_ramp_t stops[] = {
    {0.2f, 1.0f, 0.0f, 0.0f, 1.0f},   /* red  */
    {0.7f, 0.0f, 0.0f, 1.0f, 1.0f},   /* blue */
};
vg_lite_matrix_t mat;

memset(&grad, 0, sizeof(grad));
vg_lite_conic_grad_set(&grad, 2, stops, param,
                       VG_LITE_GRADIENT_SPREAD_PAD, 0);
vg_lite_conic_grad_update(&grad);

vg_lite_clear(fb, NULL, 0xFFFFFFFF);
vg_lite_identity(&mat);
vg_lite_conic_grad_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &mat, &grad,
                        VG_LITE_BLEND_NONE, VG_LITE_FILTER_POINT);
vg_lite_finish();
vg_lite_conic_grad_clear(&grad);
```
