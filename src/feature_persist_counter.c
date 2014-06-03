#include "pebble.h"

// This is a custom defined key for saving our count field
#define NUM_RUNS_VISITOR_PKEY 1
#define NUM_RUNS_HOME_PKEY 1
#define NUM_OUTS_PKEY 2
#define INNING_PKEY 3
#define TOP_INNING_PKEY 4

// You can define defaults for values in persistent storage
#define NUM_RUNS_VISITOR_DEFAULT 0
#define NUM_RUNS_HOME_DEFAULT 0
#define NUM_OUTS_DEFAULT 0
#define INNING_DEFAULT 1
#define TOP_INNING_DEFAULT 1

static Window *window;

static GBitmap *action_icon_plus;
static GBitmap *action_icon_minus;

static ActionBarLayer *action_bar;

static TextLayer *header_text_layer;
static TextLayer *body_text_layer;
static TextLayer *label_text_layer;
static TextLayer *inning_text_layer;

// We'll save the count in memory from persistent storage
static int num_runs_visitor = NUM_RUNS_VISITOR_DEFAULT;
static int num_runs_home = NUM_RUNS_HOME_DEFAULT;
static int num_outs = NUM_OUTS_DEFAULT;
static int inning = INNING_DEFAULT;
static int top_inning = TOP_INNING_DEFAULT;
static int num_inning_runs_visitor [30];
static int num_inning_runs_home[30];

static void update_text() {
  static char body_text[50];
  snprintf(body_text, sizeof(body_text), "Visitor: %u\nHome: %u", num_runs_visitor, num_runs_home);
  text_layer_set_text(body_text_layer, body_text);
}

static void update_boxscore() {
  static int boxscorelen = 120;
  static char boxscore_text[120];
  char * myptr = boxscore_text;
  int position;
  position = snprintf(myptr, boxscorelen, "   1  2  3  4  5  6  7  8  9\nV");
  position += snprintf(myptr+position, boxscorelen-position, " 1  0  5  3\nH 0 0  5  4");

/*  if(top_inning){
  snprintf(inning_text, sizeof(inning_text), "%u^", inning);
  }
  else{
  snprintf(inning_text, sizeof(inning_text), "%uv", inning);
  }
  text_layer_set_text(header_text_layer, boxscore_text);
  */
}


static void update_inning() {
  static char inning_text[3];
  if(top_inning){
  snprintf(inning_text, sizeof(inning_text), "%u^", inning);
  }
  else{
  snprintf(inning_text, sizeof(inning_text), "%uv", inning);
  }
  text_layer_set_text(inning_text_layer, inning_text);
}

static void update_outs() {
  static char outs_text[8];
  snprintf(outs_text, sizeof(outs_text), "OUTS: %u", num_outs);
  text_layer_set_text(label_text_layer, outs_text);
}

static void increment_run_handler(ClickRecognizerRef recognizer, void *context) {
  if (top_inning) {
    num_runs_visitor++;
  }
  else {
    num_runs_home++;
  }
  update_text();
}

static void decrement_run_handler(ClickRecognizerRef recognizer, void *context) {
  if (top_inning) {
    if (num_runs_visitor <= 0) {
    // Keep the counter at zero
    return;
    }

    num_runs_visitor--;
  }
  else{
        if (num_runs_home <= 0) {
    // Keep the counter at zero
    return;
  }
  

    num_runs_home--;
  }
  update_text();
}

static void increment_out_handler(ClickRecognizerRef recognizer, void *context) {
  if (num_outs == 2) {
    num_outs=0;
    if (top_inning) {
      top_inning=0;
    }
    else{
      top_inning=1;
      inning++;
    }
  }
  else {
    num_outs++;
  }
  update_inning();
  update_outs();
}

static void decrement_out_handler(ClickRecognizerRef recognizer, void *context) {
  if (num_outs == 0) {
    if (!(top_inning && inning==1)){
      if (top_inning) {
        top_inning=0;
        inning--;
      }
      else{
        top_inning=1;
      }
      num_outs=2;
    }
  }
  else {
    num_outs--;
  }
  update_inning();
  update_outs();
}

static void reset_handler(ClickRecognizerRef recognizer, void *context) {
  num_runs_visitor = 0;
  num_runs_home = 0;
  num_outs = 0;
  top_inning = 1;
  inning = 1;
  update_text();
  update_inning();
  update_outs();

}


static void click_config_provider(void *context) {
  //const uint16_t repeat_interval_ms = 50;
  //window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, (ClickHandler) increment_run_handler);
  //window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, (ClickHandler) decrement_run_handler);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) increment_run_handler);
  window_long_click_subscribe(BUTTON_ID_UP, 0, (ClickHandler) decrement_run_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) increment_out_handler);
  window_long_click_subscribe(BUTTON_ID_DOWN, 0, (ClickHandler) decrement_out_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, (ClickHandler) reset_handler, NULL);
}

static void window_load(Window *me) {
  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, me);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_plus);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_minus);

  Layer *layer = window_get_root_layer(me);
  const int16_t width = layer_get_frame(layer).size.w - ACTION_BAR_WIDTH - 3;

  header_text_layer = text_layer_create(GRect(4, 0, width, 60));
  text_layer_set_font(header_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_background_color(header_text_layer, GColorClear);
  text_layer_set_text(header_text_layer, "   1  2  3  4  5  6  7  8  9\nV 1  0  5  3\nH 0  0  5  5");
  layer_add_child(layer, text_layer_get_layer(header_text_layer));

  body_text_layer = text_layer_create(GRect(4, 50, width-28, 60));
  text_layer_set_font(body_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(body_text_layer, GColorClear);
  text_layer_set_text_alignment(body_text_layer, GTextAlignmentRight);
  layer_add_child(layer, text_layer_get_layer(body_text_layer));

  label_text_layer = text_layer_create(GRect(4, 50 + 56, width, 60));
  text_layer_set_font(label_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(label_text_layer, GColorClear);
  text_layer_set_text(label_text_layer, "OUT: 2");
  layer_add_child(layer, text_layer_get_layer(label_text_layer));

  inning_text_layer = text_layer_create(GRect(width-20, 50 + 16, 24, 28));
  text_layer_set_font(inning_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(inning_text_layer, GColorClear);
  text_layer_set_text(inning_text_layer, "3");
  layer_add_child(layer, text_layer_get_layer(inning_text_layer));

  update_text();
  update_inning();
  update_outs();
}

static void window_unload(Window *window) {
  text_layer_destroy(header_text_layer);
  text_layer_destroy(body_text_layer);
  text_layer_destroy(label_text_layer);
  text_layer_destroy(inning_text_layer);

  action_bar_layer_destroy(action_bar);
}

static void init(void) {
  action_icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);
  action_icon_minus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_MINUS);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  // Get the count from persistent storage for use if it exists, otherwise use the default
  num_runs_visitor = persist_exists(NUM_RUNS_VISITOR_PKEY) ? persist_read_int(NUM_RUNS_VISITOR_PKEY) : NUM_RUNS_VISITOR_DEFAULT;
  num_runs_home = persist_exists(NUM_RUNS_HOME_PKEY) ? persist_read_int(NUM_RUNS_HOME_PKEY) : NUM_RUNS_HOME_DEFAULT;
  num_outs = persist_exists(NUM_OUTS_PKEY) ? persist_read_int(NUM_OUTS_PKEY) : NUM_OUTS_DEFAULT;
  inning = persist_exists(INNING_PKEY) ? persist_read_int(INNING_PKEY) : INNING_DEFAULT;
  top_inning = persist_exists(TOP_INNING_PKEY) ? persist_read_int(TOP_INNING_PKEY) : TOP_INNING_DEFAULT;

  window_stack_push(window, true /* Animated */);
}

static void deinit(void) {
  // Save the count into persistent storage on app exit
  persist_write_int(NUM_RUNS_VISITOR_PKEY, num_runs_visitor);
  persist_write_int(NUM_RUNS_HOME_PKEY, num_runs_home);
  persist_write_int(NUM_OUTS_PKEY, num_outs);
  persist_write_int(INNING_PKEY, inning);
  persist_write_int(TOP_INNING_PKEY, top_inning);

  window_destroy(window);

  gbitmap_destroy(action_icon_plus);
  gbitmap_destroy(action_icon_minus);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
