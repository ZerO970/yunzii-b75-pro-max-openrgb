#pragma once

// Start in OpenRGB "direct" mode so host-sent per-LED colors are rendered.
// The RGB_MATRIX_OPENRGB_DIRECT enum is registered via
// quantum/rgb_matrix/animations/openrgb_direct_anim.h when OPENRGB_ENABLE=yes.
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_OPENRGB_DIRECT
