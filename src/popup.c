/*********************************************************************************************************************
/	Autore: 	Carlo Ramponi
/	Data:
*********************************************************************************************************************/

#include <gtk/gtk.h>

static gboolean delete_event(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

int main(int argc,char* argv[])
{
	gtk_init(&argc,&argv);
	GtkWidget * window;
	GdkColor color;
	color.red=0x4c4c;
	color.green=0xAFAF;
	color.blue=0x5050;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "NetBriscola");
	gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
	gtk_widget_show (window);
	PangoFontDescription *font_desc;
	gdk_color_parse ("white", &color);
	font_desc = pango_font_description_from_string("16");
	GtkWidget *label,*button,*table;
	table=gtk_table_new(3,1,TRUE);
	gtk_container_add(GTK_CONTAINER(window),table);

	label=gtk_label_new("Aspettando che l'avversario");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
	gtk_widget_show(label);

	label=gtk_label_new("faccia la sua mossa...");
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
	gtk_widget_modify_font(GTK_WIDGET(label), font_desc);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);

	button=gtk_button_new_with_label("OK");
	gtk_table_attach_defaults(GTK_TABLE(table),button,0,1,2,3);
	g_signal_connect(button,"clicked", GTK_SIGNAL_FUNC(delete_event),NULL);
	gtk_widget_show(button);

	gtk_widget_show(table);
	gtk_main();
	return 0;
}
