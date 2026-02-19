# 05 — Widget Reference

All LVGL widgets are available as XML tags. Every widget inherits all `lv_obj` attributes.

---

## lv_obj — Base Widget

All widgets inherit these attributes.

### Position & Size

| Attribute | Type | Example | Notes |
|-----------|------|---------|-------|
| `x` | coords | `"10"`, `"10%"` | Relative to parent |
| `y` | coords | `"20"`, `"50%"` | Relative to parent |
| `width` | coords | `"100"`, `"50%"`, `"content"` | |
| `height` | coords | `"40"`, `"content"` | |
| `align` | lv_align | `"center"`, `"top_left"` | See table below |
| `name` | string | `"my_btn"` | For `lv_obj_find_by_name()` |

**lv_align values:** `default`, `top_left`, `top_mid`, `top_right`, `bottom_left`, `bottom_mid`, `bottom_right`, `left_mid`, `right_mid`, `center`

### Inline Style Props

Any style property prefixed with `style_`:

```xml
<lv_obj style_bg_color="0x1E88E5"
        style_radius="8"
        style_pad_all="12"
        style_border_width="2"
        style_border_color="0xFFFFFF"/>
```

### Flags

Set as boolean attributes:

```xml
<lv_obj hidden="true"/>
<lv_obj clickable="false"/>
<lv_obj checkable="true"/>
<lv_obj scrollable="false"/>
<lv_obj overflow_visible="true"/>
```

All flags: `hidden`, `clickable`, `click_focusable`, `checkable`, `scrollable`, `scroll_elastic`, `scroll_momentum`, `scroll_one`, `scroll_chain_hor`, `scroll_chain_ver`, `scroll_on_focus`, `snappable`, `press_lock`, `event_bubble`, `event_trickle`, `state_trickle`, `gesture_bubble`, `adv_hittest`, `ignore_layout`, `floating`, `overflow_visible`, `flex_in_new_track`

### States

```xml
<lv_button checked="true"/>
<lv_button disabled="true"/>
<lv_button focused="true"/>
```

All states: `checked`, `focused`, `focus_key`, `edited`, `hovered`, `pressed`, `scrolled`, `disabled`

### Scroll

| Attribute | Values |
|-----------|--------|
| `scroll_snap_x` | `none`, `start`, `end`, `center` |
| `scroll_snap_y` | `none`, `start`, `end`, `center` |
| `scrollbar_mode` | `off`, `on`, `active`, `auto` |
| `ext_click_area` | int (px) |

---

## lv_button

```xml
<lv_button width="120" height="40" align="center">
    <lv_label text="Click me"/>
</lv_button>
```

Inherits all `lv_obj` attributes. Child widgets are placed inside. No button-specific attributes required.

---

## lv_label

```xml
<!-- Static text -->
<lv_label text="Hello World" long_mode="dots"/>

<!-- Reactive text from subject -->
<lv_label bind_text="counter" fmt="%d items"/>

<!-- i18n key -->
<lv_label translation_tag="greeting"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `text` | string | Static text content |
| `translation_tag` | string | i18n key |
| `long_mode` | lv_label_long_mode | `wrap`, `scroll`, `scroll_circular`, `clip`, `dots` |
| `bind_text` | subject | Bind text to a subject's value |
| `fmt` | string | printf-style format for `bind_text` (e.g. `"%d °C"`, `"%.2f V"`) |

---

## lv_image

```xml
<lv_image src="logo" scale_x="100" scale_y="100" rotation="0"/>
<lv_image src="LV_SYMBOL_WIFI" align="center"/>  <!-- symbol -->
<lv_image bind_src="selected_icon"/>              <!-- reactive -->
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `src` | image | Image name, file path, or LV symbol |
| `inner_align` | lv_image_align | `default`, `top_left`, `center`, `stretch`, `tile`, `contain`, `cover` |
| `rotation` | int | Degrees (0–3600, 0.1° resolution) |
| `scale_x` | int | Horizontal scale (256 = 100%) |
| `scale_y` | int | Vertical scale |
| `pivot_x` | int\|% | Rotation/scale pivot X |
| `pivot_y` | int\|% | Rotation/scale pivot Y |
| `bind_src` | subject | Bind image source reactively |

---

## lv_arc

```xml
<lv_arc mode="normal"
        bg_start_angle="135" bg_end_angle="45"
        min_value="0" max_value="100"
        value="60"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `start_angle` | int | Indicator start (degrees) |
| `end_angle` | int | Indicator end |
| `bg_start_angle` | int | Background arc start |
| `bg_end_angle` | int | Background arc end |
| `rotation` | int | Rotate entire arc |
| `min_value` | int | Minimum value |
| `max_value` | int | Maximum value |
| `value` | int | Current value |
| `mode` | lv_arc_mode | `normal`, `symmetrical`, `reverse` |
| `bind_value` | subject | Bind value reactively |

---

## lv_bar

```xml
<lv_bar mode="normal" min_value="0" max_value="100" value="70"/>
<lv_bar mode="range" start_value="20" value="80"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `min_value` | int | Minimum value |
| `max_value` | int | Maximum value |
| `value` | int | Current value |
| `value-animated` | bool | Animate value changes |
| `start_value` | int | Start value for range mode |
| `start_value-animated` | bool | Animate start value |
| `mode` | lv_bar_mode | `normal`, `symmetrical`, `range` |
| `orientation` | lv_bar_orientation | `auto`, `horizontal`, `vertical` |
| `bind_value` | subject | Bind value reactively |

---

## lv_slider

```xml
<lv_slider min_value="0" max_value="100" value="50"/>
<lv_slider mode="range" start_value="20" value="80"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `min_value` | int | Minimum |
| `max_value` | int | Maximum |
| `value` | int | Current value |
| `value-anim` | bool | Animate updates |
| `start_value` | int | Start value (range mode) |
| `start_value-anim` | bool | Animate start value |
| `mode` | lv_slider_mode | `normal`, `range`, `symmetrical` |
| `bind_value` | subject | Bind value reactively |

---

## lv_switch

```xml
<lv_switch width="80" height="40"/>
<lv_switch checked="true" bind_checked="is_enabled"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `orientation` | lv_switch_orientation | `auto`, `horizontal`, `vertical` |
| `checked` | bool | Initial state |
| `bind_checked` | subject | Bind checked state reactively |

---

## lv_checkbox

```xml
<lv_checkbox text="Enable feature" checked="true"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `text` | string | Label next to box |
| `checked` | bool | Initial checked state |

---

## lv_dropdown

```xml
<lv_dropdown options="Option A\nOption B\nOption C" selected="0"/>

<!-- With styled list -->
<lv_dropdown options="A\nB\nC">
    <lv_dropdown-list style_bg_color="0x222222"/>
</lv_dropdown>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `text` | string | Text shown instead of selected option |
| `options` | string | `\n`-separated list |
| `selected` | int | Selected index (0-based) |
| `symbol` | string | Symbol shown next to dropdown |
| `bind_value` | subject | Bind selected index reactively |

---

## lv_roller

```xml
<lv_roller options="Mon\nTue\nWed\nThu\nFri"
           selected="0"
           visible_row_count="3"/>

<!-- Infinite (looping) -->
<lv_roller options="0\n1\n2\n3\n4\n5\n6\n7\n8\n9"
           options-mode="infinite"
           selected="0"
           visible_row_count="4"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `options` | string | `\n`-separated options |
| `options-mode` | lv_roller_mode | `normal`, `infinite` |
| `selected` | int | Selected index |
| `selected-animated` | bool | Animate selection |
| `visible_row_count` | int | Visible rows |
| `bind_value` | subject | Bind selection reactively |

---

## lv_textarea

```xml
<lv_textarea text="" placeholder_text="Enter name..." one_line="true"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `text` | string | Content |
| `placeholder_text` | string | Hint when empty |
| `one_line` | bool | Single-line mode |
| `password_mode` | bool | Hide characters |
| `password_show_time` | int | ms to briefly show typed char |
| `text_selection` | bool | Enable text selection |
| `cursor_pos` | int | Cursor position |

---

## lv_tabview

```xml
<lv_tabview active="0" tab_bar_position="top">
    <lv_tabview-tab text="Overview">
        <lv_label text="Content 1"/>
    </lv_tabview-tab>
    <lv_tabview-tab text="History">
        <lv_label text="Content 2"/>
    </lv_tabview-tab>
</lv_tabview>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `active` | int | Active tab index |
| `tab_bar_position` | lv_dir | `top`, `bottom`, `left`, `right` |

---

## lv_chart

```xml
<lv_chart type="line" point_count="10" zoom_x="256" zoom_y="256">
    <lv_chart-series color="0xFF0000" axis="primary_y"/>
    <lv_chart-cursor color="0x00FF00" dir="right|bottom"/>
</lv_chart>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `type` | lv_chart_type | `none`, `line`, `bar`, `scatter` |
| `point_count` | int | Points per series |
| `zoom_x` / `zoom_y` | int | Zoom (256 = 100%) |
| `div_line_count_hor` / `div_line_count_ver` | int | Grid lines |

---

## lv_scale

```xml
<lv_scale mode="round_inner"
          total_tick_count="21"
          major_tick_every="5"
          label_show="true"
          range="0 100"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `mode` | lv_scale_mode | `horizontal_top`, `horizontal_bottom`, `vertical_left`, `vertical_right`, `round_inner`, `round_outer` |
| `total_tick_count` | int | Total tick marks |
| `major_tick_every` | int | Major tick interval |
| `label_show` | bool | Show tick labels |
| `range` | `"min max"` | Value range |
