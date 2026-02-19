# 03 — Screens

A screen is a full-display view. Its root `lv_obj_t` fills the entire display.

---

## Structure

```xml
<screen>
    <!-- Local styles (scoped to this screen) -->
    <styles>
        <style name="bg" bg_color="0x1a1a2e" bg_opa="255"/>
    </styles>

    <!-- Root object — becomes the screen's lv_obj_t -->
    <view>
        <style name="bg"/>

        <!-- All child widgets go here -->
        <lv_label text="Hello" align="center"/>
    </view>
</screen>
```

The `<view>` element is always the direct child of `<screen>`. Everything inside `<view>` becomes children of the root object.

### Optional `extends` on `<view>`

`<view>` defaults to `lv_obj`. You can extend another base type:

```xml
<view extends="lv_obj" width="320" height="240">
```

---

## Loading a Screen in C

```c
// Option A — load everything from directory first
lv_xml_load_all_from_path("A:ui/");
lv_obj_t *scr = lv_xml_create_screen("home");
lv_screen_load(scr);

// Option B — register and create individually
lv_xml_register_component_from_file("ui/screens/home.xml");
lv_obj_t *scr = lv_xml_create_screen("home");
lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_FADE_IN, 300, 0, false);
```

The string passed to `lv_xml_create_screen()` must match the filename without extension (e.g., `home` for `home.xml`).

---

## Screen Navigation from XML

Navigate between screens declaratively using events:

```xml
<!-- Navigate on button click -->
<lv_button>
    <screen_load_event trigger="clicked"
                       screen="settings"
                       anim_type="over_left"
                       duration="300"
                       delay="0"/>
    <lv_label text="Settings"/>
</lv_button>

<!-- Go back -->
<lv_button>
    <screen_load_event trigger="clicked"
                       screen="home"
                       anim_type="over_right"
                       duration="300"/>
    <lv_label text="Back"/>
</lv_button>
```

---

## Local Styles and Consts

Screens can declare their own styles and constants that are scoped locally:

```xml
<screen>
    <consts>
        <color name="header_bg" value="0x0d1117"/>
        <px    name="header_h"  value="48"/>
    </consts>

    <styles>
        <style name="header"
               bg_color="#header_bg"
               bg_opa="255"
               height="#header_h"
               pad_hor="12"/>
        <style name="body"
               bg_color="0x1a1a2e"
               bg_opa="255"/>
    </styles>

    <view>
        <style name="body"/>

        <!-- Header bar -->
        <lv_obj width="100%" height="#header_h" y="0" align="top_left">
            <style name="header"/>
            <lv_label text="My App"
                      style_text_color="0xFFFFFF"
                      align="left_mid" x="12"/>
        </lv_obj>

        <!-- Content area -->
        <lv_obj width="100%" y="#header_h" height="content">
            <lv_label text="Content goes here" align="top_left" x="12" y="12"/>
        </lv_obj>
    </view>
</screen>
```

---

## Screen with Data Binding

Screens can bind directly to global subjects:

```xml
<screen>
    <view style_layout="flex" style_flex_flow="column"
          style_bg_color="0x1a1a2e" style_bg_opa="255"
          style_pad_all="16" style_pad_row="8">

        <!-- Temperature display — updates automatically -->
        <lv_label bind_text="temperature" fmt="Temp: %d °C"
                  style_text_color="0xFFFFFF"
                  style_text_font="font_big"/>

        <!-- Status badge — turns red when error_code != 0 -->
        <lv_obj width="content" height="content" style_pad_all="8" style_radius="4">
            <bind_style name="error_style" subject="error_code" ref_value="1"/>
            <lv_label bind_text="status_msg" style_text_color="0xFFFFFF"/>
        </lv_obj>
    </view>
</screen>
```

---

## Full Example — Settings Screen

```xml
<!-- screens/settings.xml -->
<screen>
    <styles>
        <style name="bg"  bg_color="0x1a1a2e" bg_opa="255"/>
        <style name="row" bg_color="0x252536" radius="8" pad_all="12"/>
    </styles>

    <view>
        <style name="bg"/>

        <!-- Title -->
        <lv_label text="Settings"
                  style_text_color="0xFFFFFF"
                  style_text_font="font_big"
                  align="top_left" x="16" y="16"/>

        <!-- Scrollable content list -->
        <lv_obj width="100%" y="56" height="content"
                style_layout="flex" style_flex_flow="column"
                style_pad_all="16" style_pad_row="8">

            <!-- Wi-Fi toggle row -->
            <lv_obj width="100%" height="content">
                <style name="row"/>
                <lv_label text="Wi-Fi"
                          style_text_color="0xCCCCCC"
                          align="left_mid"/>
                <lv_switch align="right_mid"
                           bind_checked="wifi_enabled"/>
            </lv_obj>

            <!-- Brightness slider row -->
            <lv_obj width="100%" height="content">
                <style name="row"/>
                <lv_label text="Brightness"
                          style_text_color="0xCCCCCC"
                          align="left_mid"/>
                <lv_slider min_value="0" max_value="100"
                           width="120" align="right_mid"
                           bind_value="brightness"/>
            </lv_obj>

            <!-- Back button -->
            <lv_button align="center" width="120" height="40">
                <screen_load_event trigger="clicked"
                                   screen="home"
                                   anim_type="over_right"
                                   duration="250"/>
                <lv_label text="Back"/>
            </lv_button>

        </lv_obj>
    </view>
</screen>
```
