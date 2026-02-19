# 06 — Styles

LVGL XML provides three ways to apply styles: inline, named, and conditional bindings.

---

## Inline Style Properties

Any LVGL style property can be set directly on a widget using the `style_` prefix:

```xml
<lv_button style_bg_color="0x1E88E5"
           style_bg_grad_color="0x1565C0"
           style_bg_grad_dir="ver"
           style_radius="8"
           style_shadow_width="6"
           style_shadow_color="0x1E88E5"
           style_shadow_opa="80"
           style_text_color="0xFFFFFF"
           style_pad_hor="16"
           style_pad_ver="8"/>
```

---

## Named Styles

### Defining

```xml
<styles>
    <style name="card"
           bg_color="0x252536"
           radius="12"
           pad_all="16"
           shadow_width="8"
           shadow_color="0x000000"
           shadow_opa="60"/>

    <style name="btn_primary"
           bg_color="0x1E88E5"
           radius="6"
           text_color="0xFFFFFF"
           pad_hor="16"
           pad_ver="8"/>

    <style name="btn_primary_pressed"
           bg_color="0x1565C0"/>
</styles>
```

### Applying — Inline Attribute

```xml
<lv_obj styles="card"/>
<lv_button styles="btn_primary"/>
```

### Applying — Child Elements (for state-specific styles)

```xml
<lv_button>
    <style name="btn_primary"/>
    <style name="btn_primary_pressed" selector="pressed"/>
    <style name="btn_disabled"        selector="disabled"/>
    <lv_label text="Save"/>
</lv_button>

<lv_slider>
    <style name="knob_normal"  selector="knob"/>
    <style name="knob_pressed" selector="pressed|knob"/>
</lv_slider>
```

---

## Style Selectors

Apply a style only to a specific **part** and/or **state**, combined with `|`.

**Parts:** `main`, `scrollbar`, `indicator`, `knob`, `selected`, `cursor`, `items`

**States:** `default`, `pressed`, `checked`, `hovered`, `scrolled`, `disabled`, `focused`, `focus_key`, `edited`

```xml
<!-- Scrollbar style -->
<lv_obj>
    <style name="scrollbar_style" selector="scrollbar"/>
</lv_obj>

<!-- Slider indicator (filled portion) -->
<lv_slider>
    <style name="filled_track" selector="indicator"/>
</lv_slider>

<!-- Checkbox tick mark -->
<lv_checkbox>
    <style name="check_indicator" selector="indicator"/>
</lv_checkbox>
```

---

## Conditional Style Binding

Apply a style dynamically based on a subject value:

```xml
<!-- Apply "error_bg" when error_code == 1 -->
<lv_obj>
    <bind_style name="error_bg" subject="error_code" ref_value="1"/>
</lv_obj>

<!-- Apply "warning_style" when warning_active == 1 -->
<lv_label>
    <bind_style name="warning_style" subject="warning_active" ref_value="1"/>
</lv_label>
```

---

## Complete Style Property Reference

### Position & Size

| Property | Type | Notes |
|----------|------|-------|
| `x`, `y` | coords | px or % |
| `width`, `height` | coords | px, %, or `content` |
| `min_width`, `max_width` | coords | |
| `min_height`, `max_height` | coords | |

### Padding & Margin

| Property | Type |
|----------|------|
| `pad_top`, `pad_bottom`, `pad_left`, `pad_right` | int |
| `pad_hor`, `pad_ver`, `pad_all` | int |
| `pad_row`, `pad_column` | int (layout gap) |
| `margin_top`, `margin_bottom`, `margin_left`, `margin_right` | int |
| `margin_hor`, `margin_ver`, `margin_all` | int |

### Background

| Property | Type |
|----------|------|
| `bg_color` | color |
| `bg_opa` | opa |
| `bg_grad_dir` | `none`, `hor`, `ver` |
| `bg_grad_color` | color |
| `bg_main_stop` | int (0–255) |
| `bg_grad_stop` | int (0–255) |
| `bg_image_src` | image |
| `bg_image_tiled` | bool |
| `bg_image_recolor` | color |
| `bg_image_recolor_opa` | opa |
| `bg_grad` | gradient name |

### Border & Outline

| Property | Type |
|----------|------|
| `border_color` | color |
| `border_width` | int |
| `border_opa` | opa |
| `border_side` | `none`, `left`, `right`, `top`, `bottom`, `full` |
| `border_post` | bool |
| `outline_color` | color |
| `outline_width` | int |
| `outline_opa` | opa |
| `outline_pad` | int |

### Shadow

| Property | Type |
|----------|------|
| `shadow_width` | int |
| `shadow_color` | color |
| `shadow_opa` | opa |
| `shadow_offset_x` | int |
| `shadow_offset_y` | int |
| `shadow_spread` | int |

### Text

| Property | Type |
|----------|------|
| `text_color` | color |
| `text_opa` | opa |
| `text_font` | font name |
| `text_align` | `left`, `right`, `center`, `auto` |
| `text_letter_space` | int |
| `text_line_space` | int |
| `text_decor` | `none`, `underline`, `strikethrough` |

### Arc & Line

| Property | Type |
|----------|------|
| `arc_width` | int |
| `arc_color` | color |
| `arc_opa` | opa |
| `arc_rounded` | bool |
| `arc_image_src` | image |
| `line_width` | int |
| `line_color` | color |
| `line_opa` | opa |
| `line_dash_width` | int |
| `line_dash_gap` | int |
| `line_rounded` | bool |

### Transform & Misc

| Property | Type | Notes |
|----------|------|-------|
| `radius` | int | Corner radius |
| `clip_corner` | bool | |
| `opa` | opa | Overall opacity |
| `opa_layered` | opa | Snapshot blend |
| `blend_mode` | `normal`, `additive`, `subtractive`, `multiply`, `difference` | |
| `transform_scale_x` | int | 256 = 100% |
| `transform_scale_y` | int | |
| `transform_rotation` | int | 0.1° units |
| `transform_pivot_x`, `transform_pivot_y` | int | |
| `translate_x`, `translate_y` | int | Post-layout offset |
| `anim_duration` | int | ms |
| `recolor` | color | Tint |
| `recolor_opa` | opa | Tint intensity |

### Layout

| Property | Values |
|----------|--------|
| `layout` | `flex`, `grid` |
| `flex_flow` | `row`, `row_wrap`, `row_reverse`, `column`, `column_wrap`, ... |
| `flex_main_place` | `start`, `end`, `center`, `space_around`, `space_between`, `space_evenly` |
| `flex_cross_place` | same as above |
| `flex_track_place` | same as above |
| `flex_grow` | int (per-child) |

---

## Gradients

### Simple Two-Color Gradient (via style properties)

```xml
<styles>
    <style name="grad_header"
           bg_color="0x1a1a2e"
           bg_grad_color="0x0d1117"
           bg_grad_dir="ver"
           bg_main_stop="0"
           bg_grad_stop="255"/>
</styles>
```

### Named Multi-Stop Gradient

Define in the component or screen `<gradients>` block:

```xml
<gradients>
    <horizontal_gradient name="sunset">
        <stop color="0xFF6B6B" frac="0"   opa="100%"/>
        <stop color="0xFF9800" frac="128" opa="100%"/>
        <stop color="0xFFEB3B" frac="255" opa="100%"/>
    </horizontal_gradient>

    <vertical_gradient name="deep_ocean">
        <stop color="0x0277BD" frac="0%"   opa="100%"/>
        <stop color="0x01579B" frac="50%"  opa="100%"/>
        <stop color="0x006064" frac="100%" opa="100%"/>
    </vertical_gradient>
</gradients>

<styles>
    <style name="sunset_bg"   bg_grad="sunset"/>
    <style name="ocean_panel" bg_grad="deep_ocean"/>
</styles>
```

Usage:

```xml
<lv_obj width="320" height="60" styles="sunset_bg"/>
<lv_obj width="200" height="200" styles="ocean_panel" style_radius="12"/>
```

---

## Opacity Values

| Value | Meaning |
|-------|---------|
| `255` or `cover` | Fully opaque |
| `0` or `transp` | Fully transparent |
| `128` | ~50% opacity |

---

## Color Values

Hex RGB only: `0xRRGGBB`

```xml
style_bg_color="0xFF5722"    <!-- Deep Orange -->
style_text_color="0xFFFFFF"  <!-- White -->
style_border_color="0x1E88E5"<!-- Blue -->
```
