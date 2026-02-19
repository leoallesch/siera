# 04 — Components

Components are reusable widget trees with a typed, documented API. Once registered, a component is used just like a built-in widget tag.

---

## Structure

```xml
<component>
    <consts>  <!-- local constants --></consts>
    <gradients>  <!-- local gradient definitions --></gradients>
    <api>     <!-- typed props that callers pass --></api>
    <styles>  <!-- local named styles --></styles>
    <view extends="lv_obj" ...>
        <!-- widget tree -->
    </view>
</component>
```

---

## API Props

Declare every configurable parameter in `<api>`:

```xml
<api>
    <prop name="title"    type="string"   default="Untitled" help="Card title"/>
    <prop name="value"    type="int"      default="0"        help="Numeric value"/>
    <prop name="accent"   type="color"    default="0x1E88E5" help="Accent colour"/>
    <prop name="width"    type="coords"                      help="Card width"/>
    <prop name="on_click" type="event_cb"                    help="Click handler"/>
    <prop name="data"     type="subject"                     help="Bound subject"/>
</api>
```

### Prop Types

| `type=` | Accepts |
|---------|---------|
| `string` | Any text |
| `int` | Integer |
| `float` | Floating-point |
| `color` | `0xRRGGBB` |
| `coords` | px, %, `content` |
| `bool` | `true` / `false` |
| `image` | Registered image name |
| `font` | Registered font name |
| `subject` | Registered subject name |
| `event_cb` | Registered callback name |
| `style` | Name of a registered style |
| `enum:X` | One of the values in enumdef `X` |

Props with `default` are optional at the call site. Props without `default` are required.

---

## Reference Syntax Inside a Component

| Syntax | Meaning |
|--------|---------|
| `$prop_name` | Value of API prop `prop_name` |
| `#const_name` | Value of a `<consts>` constant |

```xml
<lv_label text="$title" style_text_color="$accent"/>
<lv_obj width="#size" height="content"/>
```

---

## Registering Components

```c
// From a file
lv_xml_register_component_from_file("ui/components/my_card.xml");

// Inline from a string
const char *xml = "<component><api>...</api><view extends=\"lv_obj\">...</view></component>";
lv_xml_register_component_from_data("my_card", xml);
```

---

## Using a Component as a Tag

After registration, the component's filename (without extension) is the tag name:

```xml
<my_card title="Temperature" value="23" accent="0xFF6B6B" width="200"/>
```

---

## Passing Style Parameters

A component can accept a named style from the caller:

```xml
<!-- In component API -->
<prop name="active_style" type="style"/>

<!-- In component view -->
<lv_button>
    <style name="$active_style" selector="pressed"/>
</lv_button>
```

Caller passes the style name they want:

```xml
<my_button active_style="btn_danger"/>
```

---

## Enums in Component API

```xml
<api>
    <enumdef name="size_variant">
        <enum name="small"  help="Compact layout"/>
        <enum name="medium" help="Standard layout"/>
        <enum name="large"  help="Expanded layout"/>
    </enumdef>
    <prop name="size" type="enum:size_variant" default="medium"/>
</api>
```

---

## Tutorial: Simple Label+Icon Badge

```xml
<!-- components/status_badge.xml -->
<component>
    <api>
        <prop name="icon"   type="image"  help="Icon image name"/>
        <prop name="label"  type="string" default="" help="Badge text"/>
        <prop name="color"  type="color"  default="0x4CAF50" help="Background colour"/>
    </api>

    <styles>
        <style name="bg"
               bg_color="$color"
               radius="20"
               pad_hor="8"
               pad_ver="4"/>
        <style name="text"
               text_color="0xFFFFFF"
               text_font="font_small"/>
    </styles>

    <view extends="lv_obj" width="content" height="content" styles="bg"
          style_layout="flex" style_flex_flow="row"
          style_flex_cross_place="center" style_pad_column="4">
        <lv_image src="$icon" width="16" height="16"/>
        <lv_label text="$label" styles="text"/>
    </view>
</component>
```

Usage:

```xml
<status_badge icon="icon_ok"  label="Online"  color="0x4CAF50"/>
<status_badge icon="icon_err" label="Error"   color="0xCF6679"/>
<status_badge icon="icon_warn" label="Low bat" color="0xFF9800"/>
```

---

## Tutorial: Reactive Sensor Card

```xml
<!-- components/sensor_card.xml -->
<component>
    <api>
        <prop name="title"   type="string"  help="Sensor name"/>
        <prop name="unit"    type="string"  default="" help="Unit suffix"/>
        <prop name="subject" type="subject" help="Observable value"/>
        <prop name="min_val" type="int"     default="0"   help="Arc min"/>
        <prop name="max_val" type="int"     default="100" help="Arc max"/>
        <prop name="color"   type="color"   default="0x1E88E5" help="Accent"/>
    </api>

    <styles>
        <style name="card"
               bg_color="0x0f3460"
               radius="16"
               pad_all="12"
               shadow_width="10"
               shadow_color="0x000000"
               shadow_opa="80"/>
        <style name="title_lbl"
               text_color="0x8899AA"
               text_font="font_small"/>
        <style name="value_lbl"
               text_color="0xFFFFFF"
               text_font="font_big"/>
        <style name="arc_style"
               arc_color="$color"/>
    </styles>

    <view extends="lv_obj" width="160" height="180" styles="card">

        <lv_label text="$title"
                  styles="title_lbl"
                  align="top_mid" y="4"/>

        <lv_arc bind_value="$subject"
                min_value="$min_val" max_value="$max_val"
                bg_start_angle="135" bg_end_angle="45"
                width="110" height="110"
                align="center">
            <style name="arc_style" selector="indicator"/>
        </lv_arc>

        <lv_label bind_text="$subject" fmt="%d$unit"
                  styles="value_lbl"
                  align="center"/>
    </view>
</component>
```

Usage:

```xml
<sensor_card title="Temperature" unit="°C" subject="temperature"
             min_val="-20" max_val="60" color="0xFF6B6B"/>

<sensor_card title="Humidity" unit="%" subject="humidity"
             min_val="0" max_val="100" color="0x4FC3F7"/>
```

---

## Tutorial: Navigation Button Component

```xml
<!-- components/nav_button.xml -->
<component>
    <api>
        <prop name="label"      type="string" help="Button label"/>
        <prop name="icon"       type="image"  help="Icon image"/>
        <prop name="target_scr" type="string" help="Screen name to load"/>
        <prop name="active"     type="bool"   default="false" help="Highlight as current"/>
    </api>

    <styles>
        <style name="normal"  bg_color="0x1e1e2e" radius="12" pad_all="10"/>
        <style name="active"  bg_color="0x1E88E5"/>
        <style name="pressed" bg_color="0x1565C0"/>
        <style name="lbl"     text_color="0xAAAAAA" text_font="font_small"/>
        <style name="lbl_act" text_color="0xFFFFFF"/>
    </styles>

    <view extends="lv_button" width="64" height="64" styles="normal">
        <style name="pressed" selector="pressed"/>

        <screen_load_event trigger="clicked"
                           screen="$target_scr"
                           anim_type="fade_in"
                           duration="200"/>

        <lv_image src="$icon" align="top_mid" y="8"/>

        <lv_label text="$label"
                  styles="lbl"
                  align="bottom_mid" y="-6"/>
    </view>
</component>
```

Usage:

```xml
<lv_obj style_layout="flex" style_flex_flow="row"
        style_flex_main_place="space_evenly"
        width="100%" height="content" align="bottom_mid">
    <nav_button label="Home"     icon="home_icon"     target_scr="home"/>
    <nav_button label="Settings" icon="settings_icon" target_scr="settings"/>
</lv_obj>
```

---

## Tutorial: Parameterised Modal Dialog

```xml
<!-- components/confirm_dialog.xml -->
<component>
    <api>
        <prop name="title"      type="string" default="Confirm"/>
        <prop name="message"    type="string" default="Are you sure?"/>
        <prop name="on_confirm" type="event_cb" help="Called on confirm"/>
        <prop name="on_cancel"  type="event_cb" help="Called on cancel"/>
    </api>

    <styles>
        <style name="overlay"
               bg_color="0x000000" bg_opa="160"
               radius="0"/>
        <style name="dialog"
               bg_color="0x2c2c3e"
               radius="12"
               pad_all="20"
               shadow_width="20"
               shadow_color="0x000000"
               shadow_opa="100"/>
        <style name="btn_row"
               style_layout="flex"
               style_flex_flow="row"
               style_flex_main_place="end"
               style_pad_column="8"/>
        <style name="btn_cancel"
               bg_color="0x444455" radius="6"
               text_color="0xFFFFFF"
               pad_hor="16" pad_ver="8"/>
        <style name="btn_confirm"
               bg_color="0x1E88E5" radius="6"
               text_color="0xFFFFFF"
               pad_hor="16" pad_ver="8"/>
    </styles>

    <!-- Full-screen overlay -->
    <view extends="lv_obj" width="100%" height="100%" styles="overlay">

        <!-- Centered dialog box -->
        <lv_obj width="260" height="content" align="center"
                styles="dialog"
                style_layout="flex" style_flex_flow="column"
                style_pad_row="12">

            <lv_label text="$title"
                      style_text_color="0xFFFFFF"
                      style_text_font="font_ui"/>

            <lv_label text="$message"
                      style_text_color="0xAAAAAA"
                      long_mode="wrap"
                      width="220"/>

            <!-- Button row -->
            <lv_obj width="100%" height="content" styles="btn_row">
                <lv_button styles="btn_cancel">
                    <event_cb callback="$on_cancel" trigger="clicked"/>
                    <lv_label text="Cancel"/>
                </lv_button>
                <lv_button styles="btn_confirm">
                    <event_cb callback="$on_confirm" trigger="clicked"/>
                    <lv_label text="Confirm"/>
                </lv_button>
            </lv_obj>

        </lv_obj>
    </view>
</component>
```

C registration:

```c
lv_xml_register_event_cb(NULL, "on_delete_confirm", handle_delete_confirm);
lv_xml_register_event_cb(NULL, "on_delete_cancel",  handle_delete_cancel);
```

Usage:

```xml
<confirm_dialog title="Delete item"
                message="This action cannot be undone."
                on_confirm="on_delete_confirm"
                on_cancel="on_delete_cancel"/>
```
