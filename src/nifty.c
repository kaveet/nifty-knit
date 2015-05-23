#include "pebble.h"

// This is a custom defined key for saving our count field
#define NUM_ROWS_PKEY 1
#define NUM_REPS_PKEY 2

// You can define defaults for values in persistent storage
#define NUM_ROWS_DEFAULT 0
#define NUM_REPS_DEFAULT 0

static Window *window;

static GBitmap *action_icon_plus;
static GBitmap *action_icon_minus;
static GBitmap *action_icon_reset;

static ActionBarLayer *action_bar;

static TextLayer *row_count_layer;
static TextLayer *pat_count_layer;
static TextLayer *label_text_layer;
static TextLayer *row_title_text_layer;
static TextLayer *pat_title_text_layer;

 // Vibe pattern: ON for 200ms:
static const uint32_t const segments[] = { 100 };
VibePattern pat = {
  .durations = segments,
  .num_segments = ARRAY_LENGTH(segments),
};

// We'll save the count in memory from persistent storage
static int num_rows = NUM_ROWS_DEFAULT;
static int num_reps = NUM_REPS_DEFAULT;

static void update_text_rows() {  
  static char rows_text[50];
  snprintf(rows_text, sizeof(rows_text), "%d", num_rows);
  text_layer_set_text(row_count_layer, rows_text);
}

static void update_text_reps() {
  static char reps_text[50];
  snprintf(reps_text, sizeof(reps_text), "%d", num_reps);
  text_layer_set_text(pat_count_layer, reps_text);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  num_rows++;
  update_text_rows();
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {  
  if (num_rows <= 0) {
    // Keep the counter at zero
    return;
  }
  
  vibes_enqueue_custom_pattern(pat);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_minus);
  num_rows--;
  update_text_rows();
}

static void up_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_plus);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  num_rows = 0;
  num_reps = 0;
  update_text_rows();
  update_text_reps();
  vibes_enqueue_custom_pattern(pat);
}

static void select_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {  
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  num_reps++;
  update_text_reps();
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (num_reps <= 0) {
    // Keep the counter at zero
    return;
  }
  
  vibes_enqueue_custom_pattern(pat);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_minus);
  num_reps--;
  update_text_reps();
}

static void down_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_plus);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, up_long_click_release_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
  window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_handler, down_long_click_release_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, select_long_click_release_handler);
}

static void window_load(Window *me) {
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, me);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_plus);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_plus);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_reset);

  Layer *layer = window_get_root_layer(me);
  const int16_t width = layer_get_frame(layer).size.w - ACTION_BAR_WIDTH - 3;
  
  row_title_text_layer = text_layer_create(GRect(8, 8, width, 16));
  text_layer_set_font(row_title_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(row_title_text_layer, GColorClear);
  text_layer_set_text(row_title_text_layer, "ROW");
  layer_add_child(layer, text_layer_get_layer(row_title_text_layer));

  row_count_layer = text_layer_create(GRect(8, 18, width, 64));
  text_layer_set_font(row_count_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  text_layer_set_background_color(row_count_layer, GColorClear);
  layer_add_child(layer, text_layer_get_layer(row_count_layer));
  
  pat_title_text_layer = text_layer_create(GRect(8, 86, width, 94));
  text_layer_set_font(pat_title_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(pat_title_text_layer, GColorClear);
  text_layer_set_text(pat_title_text_layer, "PATTERN REPEAT");
  layer_add_child(layer, text_layer_get_layer(pat_title_text_layer));
  
  pat_count_layer = text_layer_create(GRect(8, 96, width, 145));
  text_layer_set_font(pat_count_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  text_layer_set_background_color(pat_count_layer, GColorClear);
  layer_add_child(layer, text_layer_get_layer(pat_count_layer));

  update_text_rows();
  update_text_reps();
}


static void window_unload(Window *window) {
  text_layer_destroy(row_count_layer);
  text_layer_destroy(pat_count_layer);
  text_layer_destroy(label_text_layer);
  text_layer_destroy(row_title_text_layer);
  text_layer_destroy(pat_title_text_layer);

  action_bar_layer_destroy(action_bar);
}

static void init(void) {
  action_icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);
  action_icon_minus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_MINUS);
  action_icon_reset = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_RESET);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  // Get the count from persistent storage for use if it exists, otherwise use the default
  num_rows = persist_exists(NUM_ROWS_PKEY) ? persist_read_int(NUM_ROWS_PKEY) : NUM_ROWS_DEFAULT;
  num_reps = persist_exists(NUM_REPS_PKEY) ? persist_read_int(NUM_REPS_PKEY) : NUM_REPS_DEFAULT;

  window_stack_push(window, true /* Animated */);
}

static void deinit(void) {
  // Save the count into persistent storage on app exit
  persist_write_int(NUM_ROWS_PKEY, num_rows);
  persist_write_int(NUM_REPS_PKEY, num_reps);

  window_destroy(window);

  gbitmap_destroy(action_icon_plus);
  gbitmap_destroy(action_icon_minus);  
  gbitmap_destroy(action_icon_reset);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
