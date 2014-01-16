#include "psd_native_ext.h"

VALUE psd_native_blender_compose_bang(VALUE self) {
  psd_logger("debug", "Composing with native code");

  VALUE Compose = rb_const_get(
    rb_const_get(rb_cObject, rb_intern("PSD")),
    rb_intern("Compose")
  );
  
  VALUE fg = rb_iv_get(self, "@fg");
  VALUE bg = rb_iv_get(self, "@bg");

  VALUE fg_canvas = rb_funcall(fg, rb_intern("canvas"), 0);
  VALUE bg_canvas = rb_funcall(bg, rb_intern("canvas"), 0);

  VALUE fg_pixels = rb_funcall(fg_canvas, rb_intern("pixels"), 0);
  VALUE *bg_pixels = RARRAY_PTR(rb_funcall(bg_canvas, rb_intern("pixels"), 0));

  int fg_height = FIX2INT(rb_funcall(fg, rb_intern("height"), 0));
  int fg_width = FIX2INT(rb_funcall(fg, rb_intern("width"), 0));
  int bg_height = FIX2INT(rb_funcall(bg, rb_intern("height"), 0));
  int bg_width = FIX2INT(rb_funcall(bg, rb_intern("width"), 0));

  int offset_x = psd_clamp_int(
    FIX2INT(rb_funcall(fg, rb_intern("left"), 0)) -
    FIX2INT(rb_funcall(bg, rb_intern("left"), 0)),
    0,
    bg_width
  );

  int offset_y = psd_clamp_int(
    FIX2INT(rb_funcall(fg, rb_intern("top"), 0)) -
    FIX2INT(rb_funcall(bg, rb_intern("top"), 0)),
    0,
    bg_height
  );

  VALUE blending_mode = rb_intern_str(rb_funcall(rb_funcall(fg, rb_intern("node"), 0), rb_intern("blending_mode"), 0));
  VALUE options = rb_funcall(self, rb_intern("compose_options"), 0);

  int i, len, x, y, base_x, base_y;

  for (i = 0, len = (fg_height * fg_width); i < len; i++) {
    x = (i % fg_width);
    y = floor(i / fg_width);

    base_x = x + offset_x;
    base_y = y + offset_y;

    if (base_x < 0 || base_y < 0 || base_x >= bg_width || base_y >= bg_height) {
      continue;
    }

    bg_pixels[base_y * bg_width + base_x] = rb_funcall(
      Compose,
      blending_mode,
      3,
      rb_funcall(fg_canvas, rb_intern("[]"), 2, INT2FIX(x), INT2FIX(y)),
      bg_pixels[base_y * bg_width + base_x],
      options
    );
  }

  return Qnil;
}