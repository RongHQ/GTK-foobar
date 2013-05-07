/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) 2013 power_flow <power_flow@power-flow>
 * 
 * gtk-foobar is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gtk-foobar is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>

#include <glib/gi18n.h>
#include <gio/gio.h>

struct private{

	/* ANJUTA: Widgets declaration for gtk_foobar.ui - DO NOT REMOVE */
	GtkWidget* entry1;
	GtkWidget* entrybuffer1;
	GtkWidget* on_off_button;
	GtkWidget* textview1;
	GtkWidget* window;
	GtkWidget* textbuffer1;

};

static struct private* priv = NULL;

GCancellable *mainCanceller;

/* For testing propose use the local (not installed) ui file */
/* #define UI_FILE PACKAGE_DATA_DIR"/ui/gtk_foobar.ui" */
#define UI_FILE "src/gtk_foobar.ui"
#define TOP_WINDOW "window"


/* Signal handlers */
/* Note: These may not be declared static because signal autoconnection
 * only works with non-static methods
 */

/* Called when the window is closed */
void
destroy (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}

gpointer
textviewFiller(gpointer data)
{
	GError* USBerror = NULL;
	GDataInputStream* USBsinkDataInStream; 

	USBsinkDataInStream = g_data_input_stream_new (data);
	gtk_entry_buffer_set_text (priv->entrybuffer1, "Tracking!", -1);
	gtk_entry_set_icon_from_stock (priv->entry1, GTK_ENTRY_ICON_PRIMARY, "gtk-media-play");
		//while()
	
	
}

void
doToggle (GtkWidget *widget, gpointer data)
{
	static GFileInputStream* USBsinkInStream;
	
	gsize textsize;
	GFile* USBsink;
	GError* USBerror = NULL;
	
	if(gtk_toggle_button_get_active (priv->on_off_button)){
		const GdkRGBA green = {0.0,1.0,0.0,0.5};

		
		g_printf("On!");	
		gtk_text_buffer_set_text((GtkTextBuffer*)priv->textbuffer1, 
	                     	   "你好!",
		                         -1);
		gtk_widget_override_background_color(priv->textview1, 
		                                     GTK_STATE_FLAG_NORMAL,
		                                     &green);
		
		USBsink = g_file_new_for_path (gtk_entry_buffer_get_text (priv->entrybuffer1));
		USBsinkInStream = g_file_read (USBsink, NULL, &USBerror);

		if(USBerror != NULL){
			gtk_entry_buffer_set_text (priv->entrybuffer1, USBerror->message, -1);
			gtk_toggle_button_set_active((GtkToggleButton*)priv->on_off_button,
		                             FALSE);
			gtk_entry_set_icon_from_stock (priv->entry1, GTK_ENTRY_ICON_PRIMARY, "gtk-dialog-error");
			return;
		}
		
		
		g_thread_new ("Text View Filler", textviewFiller, USBsinkInStream);
		
	}
	else{
		const GdkRGBA red = {1.0,0.0,0.0,0.5};
		g_printf("Off!");
		gtk_text_buffer_set_text((GtkTextBuffer*)priv->textbuffer1, 
	                     	   "再见!",
	                     	   -1);
		gtk_widget_override_background_color(priv->textview1, 
		                                     GTK_STATE_FLAG_NORMAL,
		                                     &red);
		g_input_stream_close (USBsinkInStream, NULL, NULL);
		gtk_entry_set_icon_from_stock (priv->entry1, GTK_ENTRY_ICON_PRIMARY, "gtk-open");
		gtk_entry_buffer_set_text (priv->entrybuffer1, "/dev/ttyACM0", -1);
	}
	
	fflush(stdout);

}


static GtkWidget*
create_window (void)
{
	GtkWidget *window;
	GtkBuilder *builder;
	GError* error = NULL;

	/* Load UI from file */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_FILE, &error))
	{
		g_critical ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	/* Auto-connect signal handlers */
	gtk_builder_connect_signals (builder, NULL);

	/* Get the window object from the ui file */
	window = GTK_WIDGET (gtk_builder_get_object (builder, TOP_WINDOW));
        if (!window)
        {
                g_critical ("Widget \"%s\" is missing in file %s.",
				TOP_WINDOW,
				UI_FILE);
        }

	priv = malloc(sizeof(struct private));
	
	/* ANJUTA: Widgets initialization for gtk_foobar.ui - DO NOT REMOVE */
	priv->entry1 = GTK_WIDGET (gtk_builder_get_object(builder, "entry1"));
	priv->entrybuffer1 = GTK_WIDGET (gtk_builder_get_object(builder, "entrybuffer1"));
	priv->on_off_button = GTK_WIDGET (gtk_builder_get_object(builder, "on_off_button"));
	priv->textview1 = GTK_WIDGET (gtk_builder_get_object(builder, "textview1"));
	priv->window = GTK_WIDGET (gtk_builder_get_object(builder, "window"));
	priv->textbuffer1 = GTK_WIDGET (gtk_builder_get_object(builder, "textbuffer1"));
	
	g_object_unref (builder);
	
	return window;
}


int
main (int argc, char *argv[])
{
 	GtkWidget *window;


#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	
	gtk_init (&argc, &argv);

	window = create_window ();
	gtk_widget_show (window);
	
	gtk_main ();
	
	g_free(priv);

	return 0;
}
