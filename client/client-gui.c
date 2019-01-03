#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "client.h"

static const int result_buffer_height = 3000;
static const int result_buffer_width = 6;
static const int result_buffer_str_length = 30;

static const int main_window_width = 1200;
static const int main_window_height = 800;

static const int result_window_width = 800;
static const int result_window_height = 600;

static const char *search_column_names[] = { "date", "externalid", "number", "constant", "digits", "decimals" };
static const int search_column_count = 6;

GtkTreeView *result_list;
GtkListStore *result_list_store;
GtkTreeIter result_iter;

GtkEntry *search_term_entry_box;
GtkComboBoxText *column_combo_box;

void add_label(GtkContainer *container, const char* str){
  GtkWidget *label = gtk_label_new(str);
  gtk_container_add (container, label);
}

void add_combo_box(GtkContainer *container, GtkComboBoxText *combo_box, const char *entries[], const int entries_length, const char *label){
  GtkWidget *combo_container = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add (container, combo_container);

  add_label(GTK_CONTAINER (combo_container), label);

  for(int i = 0; i < entries_length; i++){
    gtk_combo_box_text_append(combo_box, NULL, entries[i]);
  }
  // Set first entry as active
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), 0);
  gtk_container_add (GTK_CONTAINER (combo_container), GTK_WIDGET(combo_box));
}

void add_entry_box(GtkContainer *container, GtkEntry *entry_box, const char *label){
  GtkWidget *search_entry_container = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add (container, search_entry_container);

  add_label(GTK_CONTAINER (search_entry_container), label);

  gtk_container_add (GTK_CONTAINER (search_entry_container), GTK_WIDGET(entry_box));
}

void add_list_column(GtkTreeView *view, const int column_row, const char* str){
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (
					       view,
					       -1,
					       str,
					       renderer, "text",
					       column_row,
					       // NULL to signal end of list
					       NULL);
}

void add_result_list_row(GtkListStore *store, const int result_str_length, char result[6][result_str_length]){
  gtk_list_store_append (store, &result_iter);
  gtk_list_store_set (store, &result_iter,
		      0, result[0],
		      1, result[1],
		      2, result[2],
		      3, result[3],
		      4, result[4],
		      5, result[5],
		      -1);
}

void add_list(GtkContainer *container, const GtkListStore *store, const int width, const char *column_names[]){
  GtkTreeView *list = GTK_TREE_VIEW(gtk_tree_view_new());

  // Add columns
  for(int i = 0; i < width; i++){
    add_list_column(GTK_TREE_VIEW (list), i, (char*)column_names[i]);
  }

  gtk_tree_view_set_model (GTK_TREE_VIEW (list), GTK_TREE_MODEL(store));
  gtk_container_add(container, GTK_WIDGET(list));
}

// Make this more generic
void create_result_window(const int result_height, const int result_width, const int result_str_length, char result[result_height][result_width][result_str_length]){
  GtkDialog *window = (GTK_DIALOG(gtk_dialog_new()));
  GtkContainer *window_container = GTK_CONTAINER(gtk_dialog_get_content_area(window));
  GtkScrolledWindow *scrolled_window;
  GtkListStore *store;

  // Set window size
  gtk_window_set_default_size (GTK_WINDOW(window), result_window_height, result_window_width);

  // Create scroll bar container
  scrolled_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
  gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 10);
  gtk_scrolled_window_set_policy (scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (window_container), GTK_WIDGET(scrolled_window), TRUE, TRUE, 0);

  store = gtk_list_store_new (result_width, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING, G_TYPE_STRING);
  // Add rows
  for(int i = 0; i < result_height; i++){
    add_result_list_row(store, result_str_length, result[i]);
  }

  add_list(GTK_CONTAINER(scrolled_window), store, result_width, search_column_names);

  gtk_widget_show_all (GTK_WIDGET(window));
}

static void begin_search (GtkWidget *widget, gpointer data) {
  char result_buffer[result_buffer_height][result_buffer_width][result_buffer_str_length];
  char *search_column_text = (char*)gtk_combo_box_text_get_active_text(column_combo_box);
  char *search_term_text = (char*)gtk_entry_get_text(search_term_entry_box);
  int result_height;

  result_height = request_database_search(search_column_text, search_term_text, result_buffer_height, result_buffer_width, result_buffer_str_length, result_buffer);

  if(result_height > 0){
    create_result_window(result_height, result_buffer_width, result_buffer_str_length, result_buffer);
  }
}

static void create_main_window (GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkContainer *window_container;

  GtkContainer *button_container;
  GtkWidget *button;

  // Setup window
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "SQL database search client");
  gtk_window_set_default_size (GTK_WINDOW (window), main_window_width, main_window_height);

  // Setup main container
  window_container = GTK_CONTAINER(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(window_container));

  // Add search button
  button_container = GTK_CONTAINER(gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL));
  gtk_container_add (window_container, GTK_WIDGET(button_container));
  button = gtk_button_new_with_label ("Search database");
  gtk_container_add (button_container, button);
  g_signal_connect(button, "clicked", G_CALLBACK (begin_search), NULL);

  // Add search term box
  search_term_entry_box = GTK_ENTRY(gtk_entry_new());
  add_entry_box(window_container, GTK_ENTRY(search_term_entry_box), "Search term:");

  /// Add search combo
  column_combo_box = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
  add_combo_box(window_container, column_combo_box, search_column_names, search_column_count, "Database column:");

  // Show window
  gtk_widget_show_all (window);
}

int main (int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new (NULL, G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (create_main_window), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
