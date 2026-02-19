# LVGL XML — Complete Reference Guide

LVGL v9.4+ ships an XML-based UI description system. You describe screens, components, and styles in `.xml` files; LVGL's parser instantiates the widget tree at runtime. This guide documents everything you need: syntax, file types, attributes, subjects/bindings, events, components, and C integration.

---

## Table of Contents

1. [Enabling XML Support](#1-enabling-xml-support)
2. [File Types](#2-file-types)
3. [project.xml](#3-projectxml)
4. [globals.xml](#4-globalsxml)
5. [Screen Files](#5-screen-files)
6. [Component Files](#6-component-files)
7. [Widget Reference](#7-widget-reference)
8. [Style System](#8-style-system)
9. [Subjects & Data Binding](#9-subjects--data-binding)
10. [Events](#10-events)
11. [Constants & Parameters](#11-constants--parameters)
12. [Layouts — Flex & Grid](#12-layouts--flex--grid)
13. [Flags & States](#13-flags--states)
14. [C API Integration](#14-c-api-integration)
15. [CMake Integration](#15-cmake-integration)
16. [Full Examples](#16-full-examples)

---

## 1. Enabling XML Support

In `lv_conf.h` set:

```c
#define LV_USE_XML 1
```

Initialize before creating any XML objects:

```c
lv_xml_init();
```

Deinitialize on shutdown:

```c
lv_xml_deinit();
```

---

## 2. File Types

| File | Root Tag | Purpose |
|------|----------|---------|
| `project.xml` | `<project>` | Display targets and project metadata |
| `globals.xml` | `<globals>` | Shared styles, consts, subjects, images, fonts |
| `<name>.xml` (screen) | `<screen>` | A full-screen view |
| `<name>.xml` (component) | `<component>` | Reusable widget tree with its own API |

The XML loader discovers files by walking a directory:

```c
lv_xml_load_all_from_path("path/to/ui/");
```

Within that directory the conventional layout is:

```
ui/
├── project.xml
├── globals.xml
└── screens/
    ├── main.xml
    └── settings.xml
```

---

## 3. project.xml

Declares display targets (only metadata, does not create displays).

```xml
<project name="my_app">
    <targets>
        <target name="target1">
            <display width="320" height="240" />
        </target>
    </targets>
</project>
```

---

## 4. globals.xml

Defines globally shared resources. All sections are optional.

```xml
<globals>
    <api>
        <!-- Global enum definitions (shared across all files) -->
        <enumdef name="my_colors">
            <enum name="red"   help="Red theme"/>
            <enum name="blue"  help="Blue theme"/>
        </enumdef>
    </api>

    <consts>
        <!-- Named constants — use with #name syntax -->
        <px   name="btn_width"    value="120"/>
        <px   name="btn_height"   value="40"/>
        <int  name="anim_dur"     value="300"/>
        <color name="brand_color" value="0x1E88E5"/>
        <string name="app_name"   value="MyApp"/>
    </consts>

    <styles>
        <!-- Named styles — use with styles="name" or <style name="name"/> -->
        <style name="card"
               bg_color="0xFFFFFF"
               radius="8"
               shadow_width="4"
               shadow_color="0x000000"
               shadow_opa="50"/>
    </styles>

    <subjects>
        <!-- Observable reactive values -->
        <int    name="counter"     value="0"/>
        <float  name="temperature" value="21.5"/>
        <string name="status_msg"  value="OK"/>
    </subjects>

    <images>
        <!-- Image assets -->
        <file name="logo"    src="images/logo.png"/>
        <data name="icon_ok" src="&amp;icon_ok_img"/>  <!-- C variable -->
    </images>

    <fonts>
        <!-- Font assets -->
        <bin      name="font_small" src="fonts/montserrat_12.bin"/>
        <tiny_ttf name="font_ui"    src="fonts/roboto.ttf" size="16"/>
    </fonts>
</globals>
```

---

## 5. Screen Files

A screen is a full-display view. The `<view>` element is the root `lv_obj_t` for the screen.

```xml
<screen>
    <!-- Local styles (scoped to this screen) -->
    <styles>
        <style name="bg" bg_color="0x1a1a2e"/>
    </styles>

    <!-- The root object of the screen -->
    <view>
        <!-- Apply a named style -->
        <style name="bg"/>

        <!-- Child widgets go here -->
        <lv_label text="Hello, World!" align="center"/>
    </view>
</screen>
```

**Loading a screen in C:**

```c
// Register it (if not loaded via lv_xml_load_all_from_path)
lv_xml_register_component_from_file("ui/screens/main.xml");

// Create and load it
lv_obj_t *scr = lv_xml_create_screen("main");
lv_screen_load(scr);
```

---

## 6. Component Files

Components are reusable widget trees with a typed API. They look like custom widget tags when used.

### Component Structure

```xml
<component>
    <!-- Local constants (scoped to this component) -->
    <consts>
        <px    name="size"   value="100"/>
        <color name="orange" value="0xffa020"/>
    </consts>

    <!-- Named gradients (scoped to this component or screen) -->
    <gradients>
        <horizontal_gradient name="grad1">
            <stop color="0xff0000" frac="30%"  opa="100%"/>
            <stop color="0x00ff00" frac="200"  opa="100%"/>
        </horizontal_gradient>
        <!-- Also supported: vertical_gradient -->
    </gradients>

    <!-- API: Declare parameters that callers pass in -->
    <api>
        <prop name="title"    type="string"   default="No title"  help="Card title"/>
        <prop name="value"    type="int"      default="0"         help="Numeric value"/>
        <prop name="color"    type="color"    default="0xcccccc"  help="Accent color"/>
        <prop name="size"     type="coords"                       help="Width"/>
        <prop name="on_click" type="event_cb"                     help="Click callback"/>
        <prop name="data"     type="subject"                      help="Bound subject"/>

        <!-- Pass a named style as a parameter (caller provides the style name) -->
        <prop name="btn_rel_style" type="style"/>
        <prop name="btn_pr_style"  type="style"/>

        <!-- Enum defined for this component -->
        <enumdef name="my_variant">
            <enum name="primary"   help="Primary style"/>
            <enum name="secondary" help="Secondary style"/>
        </enumdef>
        <prop name="variant" type="enum:my_variant" help="Visual variant"/>
    </api>

    <!-- Local styles for this component -->
    <styles>
        <style name="container"
               bg_color="0x2c2c3e"
               radius="12"
               pad_all="16"/>
        <style name="accent_style"
               bg_color="$color"/>     <!-- $color references the API prop -->
        <style name="grad_bg"
               bg_grad="grad1"/>       <!-- reference a gradient by name -->
    </styles>

    <!-- The view: extends a base widget type -->
    <view extends="lv_obj" width="#size" height="content" styles="container">

        <!-- $prop references a component parameter -->
        <lv_label text="$title"
                  style_text_color="$color"
                  align="top_left"/>

        <lv_label text="$value"
                  style_text_font="font_ui"
                  align="center"/>

        <!-- Pass a style parameter to a child component -->
        <my_button btn_text="$action" align="right_mid">
            <style name="$btn_rel_style"/>
            <style name="$btn_pr_style" selector="pressed"/>
        </my_button>

    </view>
</component>
```

### Using a Component

Once registered, a component is used as a tag:

```xml
<my_card title="Temperature" value="23" color="0xFF6B6B" size="200"/>
```

### Registering a Component in C

```c
// From a file
lv_xml_register_component_from_file("ui/components/my_card.xml");

// Or inline from a string
const char *xml = "<component><api>...</api><view extends=\"lv_obj\">...</view></component>";
lv_xml_register_component_from_data("my_card", xml);
```

### API Prop Types

| `type=` | Accepts |
|---------|---------|
| `string` | Any text |
| `int` | Integer |
| `float` | Floating-point number |
| `color` | `0xRRGGBB` |
| `coords` | px, %, `content` |
| `bool` | `true` / `false` |
| `image` | Registered image name |
| `font` | Registered font name |
| `subject` | Registered subject name |
| `event_cb` | Registered callback name |
| `style` | Name of a registered style (for passing styles as params) |
| `enum:X` | One of the values in enumdef `X` |

Props can have a `default` attribute — the value used when the caller omits that prop:

```xml
<prop name="title" type="string" default="Untitled"/>
<prop name="size"  type="int"    default="100"/>
```

### Component Reference Syntax

| Syntax | Meaning |
|--------|---------|
| `$prop_name` | Value of component parameter `prop_name` |
| `#const_name` | Value of a named constant from `<consts>` |

---

## 7. Widget Reference

All LVGL widgets are available as XML tags. Every widget inherits all `lv_obj` attributes.

### lv_obj (base)

All widgets inherit these attributes.

**Position & Size:**

| Attribute | Type | Example | Notes |
|-----------|------|---------|-------|
| `x` | coords | `"10"`, `"10%"` | X position relative to parent |
| `y` | coords | `"20"`, `"50%"` | Y position relative to parent |
| `width` | coords | `"100"`, `"50%"`, `"content"` | |
| `height` | coords | `"40"`, `"content"` | |
| `align` | lv_align | `"center"`, `"top_left"` | See alignment table |
| `name` | string | `"my_btn"` | For `lv_obj_find_by_name()` |

**lv_align values:** `default`, `top_left`, `top_mid`, `top_right`, `bottom_left`, `bottom_mid`, `bottom_right`, `left_mid`, `right_mid`, `center`

**Inline style props** (prefix `style_`): Any style property can be set inline — see [Style System](#8-style-system).

**Flags** (`true`/`false`):
`hidden`, `clickable`, `click_focusable`, `checkable`, `scrollable`,
`scroll_elastic`, `scroll_momentum`, `scroll_one`, `scroll_chain_hor`,
`scroll_chain_ver`, `scroll_on_focus`, `snappable`, `press_lock`,
`event_bubble`, `event_trickle`, `state_trickle`, `gesture_bubble`,
`adv_hittest`, `ignore_layout`, `floating`, `overflow_visible`, `flex_in_new_track`

**States** (`true`/`false`):
`checked`, `focused`, `focus_key`, `edited`, `hovered`, `pressed`, `scrolled`, `disabled`

**Scroll:**

| Attribute | Values |
|-----------|--------|
| `scroll_snap_x` | `none`, `start`, `end`, `center` |
| `scroll_snap_y` | `none`, `start`, `end`, `center` |
| `scrollbar_mode` | `off`, `on`, `active`, `auto` |
| `ext_click_area` | int (px) |

**Binding:**

| Attribute | Description |
|-----------|-------------|
| `bind_checked` | subject — binds checked state |

---

### lv_button

```xml
<lv_button width="120" height="40" align="center">
    <lv_label text="Click me"/>
</lv_button>
```

Inherits all `lv_obj` attributes. No button-specific attributes needed — children are placed inside.

---

### lv_label

```xml
<lv_label text="Hello World" long_mode="dots"/>
<lv_label bind_text="counter" fmt="%d items"/>
<lv_label translation_tag="greeting"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `text` | string | Static text content |
| `translation_tag` | string | i18n key (use translations system) |
| `long_mode` | lv_label_long_mode | `wrap`, `scroll`, `scroll_circular`, `clip`, `dots` |
| `bind_text` | subject | Bind text to a subject's value |
| `fmt` | string | printf-style format string for `bind_text` (e.g. `"%d°C"`) |

---

### lv_image

```xml
<lv_image src="logo" scale_x="100" scale_y="100" rotation="0"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `src` | image | Image name (from globals), file path, or symbol |
| `inner_align` | lv_image_align | `default`, `top_left`, `center`, `stretch`, `tile`, `contain`, `cover`, etc. |
| `rotation` | int | Degrees |
| `scale_x` | int | Horizontal scale (100 = original) |
| `scale_y` | int | Vertical scale (100 = original) |
| `pivot_x` | int\|% | Rotation/scale pivot X |
| `pivot_y` | int\|% | Rotation/scale pivot Y |
| `bind_src` | subject | Bind image source to a subject |

---

### lv_arc

```xml
<lv_arc mode="normal"
        bg_start_angle="135" bg_end_angle="45"
        min_value="0" max_value="100"
        value="60"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `start_angle` | int | Indicator start angle (degrees) |
| `end_angle` | int | Indicator end angle |
| `bg_start_angle` | int | Background arc start angle |
| `bg_end_angle` | int | Background arc end angle |
| `rotation` | int | Rotate the whole arc |
| `min_value` | int | Minimum value |
| `max_value` | int | Maximum value |
| `value` | int | Current value |
| `mode` | lv_arc_mode | `normal`, `symmetrical`, `reverse` |
| `bind_value` | subject | Bind value to a subject |

---

### lv_bar

```xml
<lv_bar mode="normal" min_value="0" max_value="100" value="70"/>
<lv_bar mode="range" start_value="20" value="80"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `min_value` | int | Minimum value |
| `max_value` | int | Maximum value |
| `value` | int | Current value |
| `value-animated` | bool | Animate value changes (suffix syntax) |
| `start_value` | int | Start value for range mode |
| `start_value-animated` | bool | Animate start value |
| `mode` | lv_bar_mode | `normal`, `symmetrical`, `range` |
| `orientation` | lv_bar_orientation | `auto`, `horizontal`, `vertical` |
| `bind_value` | subject | Bind value to a subject |

---

### lv_slider

```xml
<lv_slider min_value="0" max_value="100" value="50" mode="normal"/>
<lv_slider mode="range" start_value="20" value="80"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `min_value` | int | Minimum |
| `max_value` | int | Maximum |
| `value` | int | Current value |
| `value-anim` | bool | Animate value updates |
| `start_value` | int | Start value (range mode) |
| `start_value-anim` | bool | Animate start value |
| `mode` | lv_slider_mode | `normal`, `range`, `symmetrical` |
| `bind_value` | subject | Bind value to a subject |

---

### lv_switch

```xml
<lv_switch width="80" height="40"/>
<lv_switch checked="true" bind_checked="is_enabled"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `orientation` | lv_switch_orientation | `auto`, `horizontal`, `vertical` |
| `checked` | bool (flag) | Initial state |
| `bind_checked` | subject | Bind checked state (from `lv_obj`) |

---

### lv_checkbox

```xml
<lv_checkbox text="Enable feature" checked="true"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `text` | string | Label next to checkbox |
| `checked` | bool (flag) | Initial checked state |

---

### lv_dropdown

```xml
<lv_dropdown options="Option A\nOption B\nOption C" selected="0"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `text` | string | Text shown instead of selected option |
| `options` | string | `\n`-separated list of options |
| `selected` | int | Selected index (0-based) |
| `symbol` | string | Symbol shown next to dropdown |
| `bind_value` | subject | Bind selected index to a subject |

Child element for styling the list:
```xml
<lv_dropdown options="A\nB\nC">
    <lv_dropdown-list style_bg_color="0x222222"/>
</lv_dropdown>
```

---

### lv_roller

```xml
<lv_roller options="'Mon\nTue\nWed\nThu\nFri' infinite"
           selected="0"
           visible_row_count="3"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `options` | string | `\n`-separated options; append ` infinite` for looping |
| `options-mode` | lv_roller_mode | `normal`, `infinite` |
| `selected` | int | Selected index |
| `selected-animated` | bool | Animate selection |
| `visible_row_count` | int | Rows visible at once |
| `bind_value` | subject | Bind selection to a subject |

---

### lv_textarea

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
| `cursor_pos` | int | Cursor position (0-based) |

---

### lv_tabview

```xml
<lv_tabview active="0" tab_bar_position="top">
    <lv_tabview-tab text="Tab 1">
        <lv_label text="Content 1"/>
    </lv_tabview-tab>
    <lv_tabview-tab text="Tab 2">
        <lv_label text="Content 2"/>
    </lv_tabview-tab>
</lv_tabview>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `active` | int | Active tab index |
| `tab_bar_position` | lv_dir | `top`, `bottom`, `left`, `right` |

---

### lv_chart

```xml
<lv_chart type="line" point_count="10" zoom_x="256" zoom_y="256">
    <lv_chart-series color="0xFF0000" axis="primary_y"/>
    <lv_chart-cursor color="0x00FF00" dir="right|bottom"/>
</lv_chart>
```

See `lv_chart.xml` for the full attribute list. Key attributes:

| Attribute | Type | Description |
|-----------|------|-------------|
| `type` | lv_chart_type | `none`, `line`, `bar`, `scatter` |
| `point_count` | int | Points per series |
| `zoom_x` / `zoom_y` | int | Zoom (256 = 100%) |
| `div_line_count_hor` / `div_line_count_ver` | int | Grid lines |
| `range` (series child) | `"min max"` | Y-axis range |

---

### lv_scale

```xml
<lv_scale mode="round_inner" total_tick_count="21" major_tick_every="5"
          label_show="true" range="0 100"/>
```

| Attribute | Type | Description |
|-----------|------|-------------|
| `mode` | lv_scale_mode | `horizontal_top`, `horizontal_bottom`, `vertical_left`, `vertical_right`, `round_inner`, `round_outer` |
| `total_tick_count` | int | Total tick marks |
| `major_tick_every` | int | Major tick interval |
| `label_show` | bool | Show tick labels |
| `range` | `"min max"` | Value range |

---

## 8. Style System

### Inline Style Properties

Any style property can be applied directly as an attribute with the `style_` prefix:

```xml
<lv_button style_bg_color="0x1E88E5"
           style_radius="8"
           style_shadow_width="4"
           style_text_color="0xFFFFFF"
           style_pad_all="12"/>
```

### Named Styles

Define once, apply many times.

**Definition** (in `globals.xml`, screen, or component `<styles>` block):

```xml
<styles>
    <style name="btn_primary"
           bg_color="0x1E88E5"
           bg_grad_color="0x1565C0"
           bg_grad_dir="ver"
           radius="6"
           text_color="0xFFFFFF"
           pad_hor="16"
           pad_ver="8"/>
</styles>
```

**Application** — inline via `styles` attribute:

```xml
<lv_button styles="btn_primary"/>
```

**Application** — as a child element (for state-specific styles):

```xml
<lv_button>
    <style name="btn_primary"/>
    <style name="btn_pressed" selector="pressed"/>
    <style name="btn_disabled" selector="disabled"/>
</lv_button>
```

**Binding a style conditionally to a subject:**

```xml
<lv_label>
    <bind_style name="error_style" subject="error_code" ref_value="1"/>
</lv_label>
```

### Style Selectors

Selectors target a widget **part** and/or **state**, ORed together with `|`.

**Parts:** `main`, `scrollbar`, `indicator`, `knob`, `selected`, `cursor`, `items`

**States:** `default`, `pressed`, `checked`, `hovered`, `scrolled`, `disabled`, `focused`, `focus_key`, `edited`

```xml
<!-- Style the knob of a slider when pressed -->
<lv_slider>
    <style name="knob_pressed" selector="pressed|knob"/>
</lv_slider>
```

### Complete Style Property Reference

**Position & Size:**

| Property | Type | Notes |
|----------|------|-------|
| `x`, `y` | coords | px or % |
| `width`, `height` | coords | px, %, or `content` |
| `min_width`, `max_width`, `min_height`, `max_height` | coords | |

**Padding & Margin:**

| Property | Type |
|----------|------|
| `pad_top`, `pad_bottom`, `pad_left`, `pad_right` | int |
| `pad_hor`, `pad_ver`, `pad_all` | int (shorthand) |
| `pad_row`, `pad_column` | int (layout spacing) |
| `margin_top`, `margin_bottom`, `margin_left`, `margin_right` | int |
| `margin_hor`, `margin_ver`, `margin_all` | int |

**Background:**

| Property | Type |
|----------|------|
| `bg_color` | color (`0xRRGGBB`) |
| `bg_opa` | opa (0–255 or `transp`/`cover`) |
| `bg_grad_dir` | `none`, `hor`, `ver` |
| `bg_grad_color` | color |
| `bg_main_stop` | int (0–255, gradient start position) |
| `bg_grad_stop` | int (0–255, gradient end position) |
| `bg_image_src` | image |
| `bg_image_tiled` | bool |
| `bg_image_recolor` | color |
| `bg_image_recolor_opa` | opa |

**Border & Outline:**

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

**Shadow:**

| Property | Type |
|----------|------|
| `shadow_width` | int |
| `shadow_color` | color |
| `shadow_opa` | opa |
| `shadow_offset_x` | int |
| `shadow_offset_y` | int |
| `shadow_spread` | int |

**Text:**

| Property | Type |
|----------|------|
| `text_color` | color |
| `text_opa` | opa |
| `text_font` | font name |
| `text_align` | `left`, `right`, `center`, `auto` |
| `text_letter_space` | int |
| `text_line_space` | int |
| `text_decor` | `none`, `underline`, `strikethrough` |

**Arc & Line:**

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

**Transform & Misc:**

| Property | Type | Notes |
|----------|------|-------|
| `radius` | int | Corner radius |
| `clip_corner` | bool | |
| `opa` | opa | Overall opacity |
| `opa_layered` | opa | Snapshot blend |
| `blend_mode` | `normal`, `additive`, `subtractive`, `multiply`, `difference` | |
| `transform_scale_x` | int | 256 = 100% |
| `transform_scale_y` | int | 256 = 100% |
| `transform_rotation` | int | 0.1° units |
| `transform_pivot_x`, `transform_pivot_y` | int | |
| `translate_x`, `translate_y` | int | Post-layout offset |
| `anim_duration` | int | ms (e.g. label scroll speed) |
| `recolor` | color | Tint widget and children |
| `recolor_opa` | opa | Tint intensity |

### Gradients

Define a named gradient in the component/globals `<styles>` scope:

```xml
<styles>
    <style name="grad_bg" bg_grad_dir="ver"
           bg_color="0x1a1a2e"
           bg_grad_color="0x16213e"
           bg_main_stop="0"
           bg_grad_stop="255"/>
</styles>
```

---

## 9. Subjects & Data Binding

Subjects are observable reactive values (from `lv_observer`). They let the UI react to state changes without manual refresh calls.

### Declaring Subjects

In `globals.xml`:

```xml
<subjects>
    <int    name="temperature" value="20"/>
    <float  name="humidity"    value="55.0"/>
    <string name="status"      value="Ready"/>
</subjects>
```

In a component `<api>`:

```xml
<prop name="data_subject" type="subject"/>
```

### Binding Widgets to Subjects

**Bind widget value:**

```xml
<lv_arc    bind_value="temperature"/>
<lv_bar    bind_value="temperature"/>
<lv_slider bind_value="temperature"/>
<lv_label  bind_text="temperature" fmt="%.1f°C"/>
<lv_image  bind_src="selected_icon"/>
<lv_switch bind_checked="is_enabled"/>
```

**Bind checked state:**

```xml
<lv_checkbox bind_checked="feature_on"/>
```

**Bind a style conditionally:**

```xml
<!-- Apply "error_style" when error_code == 1 -->
<lv_obj>
    <bind_style name="error_style" subject="error_code" ref_value="1"/>
</lv_obj>
```

**Bind flags based on subject value:**

```xml
<!-- Hide widget when mode == 0 -->
<lv_obj>
    <bind_flag_if_eq subject="mode" flag="hidden" ref_value="0"/>
</lv_obj>

<!-- Available comparisons: -->
<bind_flag_if_eq     subject="x" flag="hidden" ref_value="5"/>
<bind_flag_if_not_eq subject="x" flag="hidden" ref_value="5"/>
<bind_flag_if_gt     subject="x" flag="hidden" ref_value="5"/>
<bind_flag_if_ge     subject="x" flag="hidden" ref_value="5"/>
<bind_flag_if_lt     subject="x" flag="hidden" ref_value="5"/>
<bind_flag_if_le     subject="x" flag="hidden" ref_value="5"/>
```

**Bind states based on subject value:**

```xml
<lv_obj>
    <bind_state_if_eq     subject="error_code" state="disabled" ref_value="1"/>
    <bind_state_if_not_eq subject="error_code" state="focused"  ref_value="0"/>
    <bind_state_if_gt     subject="level"      state="checked"  ref_value="50"/>
</lv_obj>
```

### Registering Subjects in C

```c
lv_subject_t my_temp;
lv_subject_init_int(&my_temp, 20);
lv_xml_register_subject(NULL, "temperature", &my_temp);

// Update from anywhere — UI reacts automatically
lv_subject_set_int(&my_temp, 25);
```

---

## 10. Events

### Event Callbacks

```xml
<!-- Call a C function on click -->
<lv_button>
    <event_cb callback="on_btn_click" trigger="clicked"/>
    <event_cb callback="on_btn_press" trigger="pressed" user_data="my_btn"/>
</lv_button>
```

Register the C callback before creating the screen:

```c
static void on_btn_click(lv_event_t *e) {
    const char *data = lv_event_get_user_data(e);
    /* ... */
}

lv_xml_register_event_cb(NULL, "on_btn_click", on_btn_click);
```

### Subject Manipulation on Event

```xml
<!-- Toggle a subject value on click -->
<lv_button>
    <subject_toggle_event subject="is_active" trigger="clicked"/>
</lv_button>

<!-- Set an int subject on click -->
<lv_button>
    <subject_set_int_event subject="page" trigger="clicked" value="2"/>
</lv_button>

<!-- Set a float subject on value_changed -->
<lv_slider>
    <subject_set_float_event subject="volume" trigger="value_changed" value="0.5"/>
</lv_slider>

<!-- Set a string subject -->
<lv_button>
    <subject_set_string_event subject="status" trigger="clicked" value="Done"/>
</lv_button>

<!-- Increment a subject (with rollover) -->
<lv_button>
    <subject_increment_event subject="counter" trigger="clicked" step="1">
        <rollover>true</rollover>
        <min_value>0</min_value>
        <max_value>9</max_value>
    </subject_increment_event>
</lv_button>
```

### Screen Navigation on Event

```xml
<!-- Load an existing screen on click -->
<lv_button>
    <screen_load_event trigger="clicked"
                       screen="settings"
                       anim_type="over_left"
                       duration="300"
                       delay="0"/>
</lv_button>

<!-- Create a new screen dynamically on click -->
<lv_button>
    <screen_create_event trigger="clicked"
                         screen="create_settings_screen"
                         anim_type="fade_in"
                         duration="200"/>
</lv_button>
```

**lv_screen_load_anim values:**
`none`, `over_left`, `over_right`, `over_top`, `over_bottom`,
`move_left`, `move_right`, `move_top`, `move_bottom`,
`fade_in`, `fade_on`, `fade_out`,
`out_left`, `out_right`, `out_top`, `out_bottom`

### Timeline Events

```xml
<lv_button>
    <play_timeline_event trigger="clicked"
                         target="my_animated_obj"
                         timeline="my_timeline"
                         delay="0"
                         reverse="false"/>
</lv_button>
```

### Event Trigger Values

`pressed`, `pressing`, `press_lost`, `short_clicked`, `single_clicked`,
`double_clicked`, `triple_clicked`, `long_pressed`, `long_pressed_repeat`,
`clicked`, `released`, `scroll_begin`, `scroll_end`, `scroll`,
`gesture`, `key`, `rotary`, `focused`, `defocused`, `value_changed`,
`insert`, `refresh`, `ready`, `cancel`, `create`, `delete`

---

## 11. Constants & Parameters

### Named Constants

Define in `globals.xml` or a component's `<consts>` block (not shown in component XML but accessible via parent scope). Reference with `#name`.

```xml
<!-- globals.xml -->
<consts>
    <px    name="btn_w"    value="120"/>
    <px    name="btn_h"    value="44"/>
    <color name="primary"  value="0x1E88E5"/>
    <int   name="radius"   value="8"/>
    <string name="version" value="1.0.0"/>
</consts>
```

Usage:

```xml
<lv_button width="#btn_w" height="#btn_h" style_bg_color="#primary" style_radius="#radius"/>
```

### Coordinate Value Types

| Format | Meaning |
|--------|---------|
| `100` | 100 pixels |
| `100px` | 100 pixels |
| `50%` | 50% of parent |
| `content` | Size to fit children |

### Opacity Values

| Value | Meaning |
|-------|---------|
| `255` or `cover` | Fully opaque |
| `0` or `transp` | Fully transparent |
| `128` | ~50% opacity |

### Color Values

Colors are hex RGB: `0xRRGGBB`

```xml
style_bg_color="0xFF5722"   <!-- Deep Orange -->
style_text_color="0xFFFFFF" <!-- White -->
```

---

## 12. Layouts — Flex & Grid

### Flex Layout

```xml
<lv_obj style_layout="flex"
        style_flex_flow="row"
        style_flex_main_place="space_between"
        style_flex_cross_place="center"
        style_pad_column="8"
        width="300" height="content">

    <lv_label text="Label" style_flex_grow="1"/>
    <lv_button width="80">
        <lv_label text="OK"/>
    </lv_button>
</lv_obj>
```

**flex_flow values:** `row`, `row_wrap`, `row_reverse`, `row_wrap_reverse`,
`column`, `column_wrap`, `column_reverse`, `column_wrap_reverse`

**flex_main_place / flex_cross_place / flex_track_place values:**
`start`, `end`, `center`, `space_around`, `space_between`, `space_evenly`

**Per-child:**

```xml
<lv_label style_flex_grow="1"/>          <!-- fill remaining space -->
<lv_button flex_in_new_track="true"/>    <!-- start a new row/column -->
```

### Grid Layout

```xml
<lv_obj style_layout="grid"
        style_grid_column_dsc_array="100 fr(1) 80 LV_GRID_TEMPLATE_LAST"
        style_grid_row_dsc_array="40 40 LV_GRID_TEMPLATE_LAST"
        width="300" height="content">

    <lv_label text="Name"
              style_grid_cell_column_pos="0" style_grid_cell_row_pos="0"
              style_grid_cell_x_align="start" style_grid_cell_y_align="center"/>

    <lv_textarea style_grid_cell_column_pos="1" style_grid_cell_row_pos="0"
                 style_grid_cell_column_span="2"/>

</lv_obj>
```

---

## 13. Flags & States

Set flags and states as boolean attributes directly on widgets:

```xml
<!-- Flags -->
<lv_obj hidden="true"/>
<lv_obj clickable="false"/>
<lv_obj checkable="true"/>
<lv_obj scrollable="false"/>
<lv_obj overflow_visible="true"/>

<!-- States -->
<lv_button checked="true"/>
<lv_button disabled="true"/>
<lv_button focused="true"/>
```

---

## 14. C API Integration

### Initialization

```c
#include "lvgl/lvgl.h"

// Init LVGL and display first, then:
lv_xml_init();

// Optional: set prefix for asset paths (images, fonts)
lv_xml_set_default_asset_path("A:ui/");
```

### Loading All XML from a Directory

```c
lv_result_t res = lv_xml_load_all_from_path("ui/");
if (res != LV_RESULT_OK) {
    // handle error
}
```

### Creating Widgets Programmatically

```c
// Equivalent to <lv_slider width="200" value="50"/>
const char *attrs[] = {"width", "200", "value", "50", NULL, NULL};
lv_obj_t *slider = lv_xml_create(parent, "lv_slider", attrs);

// Create a registered component
const char *card_attrs[] = {"title", "Temp", "value", "23", NULL, NULL};
lv_obj_t *card = lv_xml_create(parent, "my_card", card_attrs);
```

### Creating and Loading Screens

```c
// Create a screen from a registered XML screen
lv_obj_t *scr = lv_xml_create_screen("main");
lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0, false);
```

### Registering Resources

```c
// Register a C event callback
lv_xml_register_event_cb(NULL, "on_button_click", my_btn_handler);

// Register a subject
lv_subject_t temp_subject;
lv_subject_init_int(&temp_subject, 0);
lv_xml_register_subject(NULL, "temperature", &temp_subject);

// Register a font
lv_xml_register_font(NULL, "font_ui", &lv_font_montserrat_16);

// Register an image (C variable)
lv_xml_register_image(NULL, "logo", &my_logo_img_dsc);

// Register a named constant
lv_xml_register_const(NULL, "BRAND_COLOR", "0x1E88E5");
```

### Register a Custom Widget

```c
void * my_widget_create(lv_xml_parser_state_t *state, const char **attrs) {
    lv_obj_t *obj = lv_obj_create(lv_xml_state_get_parent(state));
    return obj;
}

void my_widget_apply(lv_xml_parser_state_t *state, const char **attrs) {
    lv_obj_t *obj = lv_xml_state_get_obj(state);
    // parse attrs and configure obj
}

lv_xml_register_widget("my_widget", my_widget_create, my_widget_apply);
```

---

## 15. CMake Integration

### Fetching LVGL with XML enabled

```cmake
set(LV_USE_XML 1)  # or set it in lv_conf.h

FetchContent_Declare(
    lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG        v9.4.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(lvgl)
```

### Including Generated UI Sources

The LVGL editor can generate C wrappers. If you use them, add them to your build:

```cmake
# file_list_gen.cmake is generated by the LVGL editor
include(${CMAKE_CURRENT_SOURCE_DIR}/ui/file_list_gen.cmake)

target_sources(my_target PRIVATE ${PROJECT_SOURCES})
```

A typical `file_list_gen.cmake` looks like:

```cmake
list(APPEND PROJECT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/screens/main_gen.c
    ${CMAKE_CURRENT_LIST_DIR}/ui_main_gen.c
    ${CMAKE_CURRENT_LIST_DIR}/ui_main.c)
```

---

## 16. Full Examples

### Simple Screen with a Styled Button

```xml
<!-- screens/home.xml -->
<screen>
    <styles>
        <style name="bg"
               bg_color="0x1a1a2e"
               bg_opa="255"/>
        <style name="btn"
               bg_color="0x1E88E5"
               radius="8"
               text_color="0xFFFFFF"
               pad_hor="20"
               pad_ver="10"
               shadow_width="8"
               shadow_color="0x1E88E5"
               shadow_opa="100"/>
    </styles>

    <view>
        <style name="bg"/>

        <lv_label text="Welcome"
                  style_text_color="0xFFFFFF"
                  style_text_font="font_ui"
                  align="top_mid"
                  y="40"/>

        <lv_button align="center">
            <style name="btn"/>
            <event_cb callback="on_start" trigger="clicked"/>
            <lv_label text="Get Started"/>
        </lv_button>
    </view>
</screen>
```

### Reactive Dashboard Component

```xml
<!-- components/sensor_card.xml -->
<component>
    <api>
        <prop name="title"   type="string" help="Sensor name"/>
        <prop name="unit"    type="string" help="Unit suffix"/>
        <prop name="subject" type="subject" help="Observable value"/>
        <prop name="min_val" type="int"    help="Arc min"/>
        <prop name="max_val" type="int"    help="Arc max"/>
    </api>

    <styles>
        <style name="card"
               bg_color="0x0f3460"
               radius="16"
               pad_all="12"
               shadow_width="10"
               shadow_color="0x000000"
               shadow_opa="80"/>
        <style name="title_style"
               text_color="0x8899AA"
               text_font="font_small"/>
        <style name="value_style"
               text_color="0xFFFFFF"
               text_font="font_big"/>
    </styles>

    <view extends="lv_obj" width="160" height="180" styles="card">
        <lv_label text="$title"
                  styles="title_style"
                  align="top_mid" y="4"/>

        <lv_arc bind_value="$subject"
                min_value="$min_val" max_value="$max_val"
                bg_start_angle="135" bg_end_angle="45"
                width="110" height="110"
                align="center"/>

        <lv_label bind_text="$subject" fmt="%d$unit"
                  styles="value_style"
                  align="center"/>
    </view>
</component>
```

Usage:

```xml
<screen>
    <view style_layout="flex" style_flex_flow="row_wrap"
          style_pad_all="16" style_pad_column="12" style_pad_row="12">

        <sensor_card title="Temperature" unit="°C"
                     subject="temperature"
                     min_val="-20" max_val="60"/>

        <sensor_card title="Humidity" unit="%"
                     subject="humidity"
                     min_val="0" max_val="100"/>
    </view>
</screen>
```

### Custom Component with Events and Navigation

```xml
<!-- components/nav_button.xml -->
<component>
    <api>
        <prop name="label"      type="string" help="Button label"/>
        <prop name="target_scr" type="screen" help="Screen to navigate to"/>
        <prop name="icon"       type="image"  help="Button icon"/>
    </api>

    <styles>
        <style name="normal" bg_color="0x222233" radius="12" pad_all="10"/>
        <style name="pressed" bg_color="0x1E88E5"/>
    </styles>

    <view extends="lv_button" width="100" height="80" styles="normal">
        <style name="pressed" selector="pressed"/>
        <screen_load_event trigger="clicked"
                           screen="$target_scr"
                           anim_type="over_left"
                           duration="250"/>

        <lv_image src="$icon" align="top_mid" y="6"/>
        <lv_label text="$label"
                  style_text_color="0xCCCCCC"
                  align="bottom_mid" y="-6"/>
    </view>
</component>
```

### Globals with Subjects and Fonts

```xml
<!-- globals.xml -->
<globals>
    <consts>
        <px    name="screen_w"    value="320"/>
        <px    name="screen_h"    value="240"/>
        <color name="primary"     value="0x1E88E5"/>
        <color name="surface"     value="0x1a1a2e"/>
        <int   name="corner_r"    value="8"/>
    </consts>

    <styles>
        <style name="surface_card"
               bg_color="#surface"
               radius="#corner_r"
               pad_all="12"/>
    </styles>

    <subjects>
        <int    name="page"        value="0"/>
        <int    name="temperature" value="0"/>
        <float  name="humidity"    value="0.0"/>
        <string name="status"      value="Idle"/>
    </subjects>

    <images>
        <file name="home_icon"     src="images/home.png"/>
        <file name="settings_icon" src="images/settings.png"/>
    </images>

    <fonts>
        <bin name="font_small" src="fonts/montserrat_12.bin"/>
        <bin name="font_ui"    src="fonts/montserrat_16.bin"/>
        <bin name="font_big"   src="fonts/montserrat_24.bin"/>
    </fonts>
</globals>
```

---

## Quick Reference Card

### Attribute Value Types

| Type | Example values |
|------|---------------|
| `int` | `0`, `42`, `-10` |
| `coords` | `100`, `50%`, `content` |
| `bool` | `true`, `false` |
| `color` | `0xFF5722` |
| `opa` | `0`–`255`, `transp`, `cover` |
| `string` | `"Hello"` |
| `font` | name registered with `lv_xml_register_font()` |
| `image` | name registered with `lv_xml_register_image()` |
| `subject` | name registered with `lv_xml_register_subject()` |
| `event_cb` | name registered with `lv_xml_register_event_cb()` |
| `enum:X` | value from enumdef `X` |

### Special Prefix Syntax

| Prefix | Usage | Meaning |
|--------|-------|---------|
| `style_` | on any widget | Inline style property |
| `#` | as attribute value | Reference a named constant |
| `$` | in component | Reference a component API parameter |

### File Loading Order

1. `globals.xml` — constants, styles, subjects, assets
2. Component XMLs — register before screens that use them
3. Screen XMLs — can now reference all the above

`lv_xml_load_all_from_path()` handles this automatically by doing multiple passes.
