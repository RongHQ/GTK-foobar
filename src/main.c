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
	GtkWidget* label2;
	GtkWidget* label3;
	GtkWidget* label1;
	GtkWidget* spinner1;
	GtkWidget* comboboxtext_entry;
	GtkWidget* comboboxtext2;
	GtkWidget* entry1;
	GtkWidget* entrybuffer1;
	GtkWidget* on_off_button;
	GtkWidget* textview1;
	GtkWidget* window;
	GtkWidget* textbuffer1;

};

static struct private* priv = NULL;

int addr_book[4] = {0x8c10, 0x8d62, 0x8bb3, 0x8de4};
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

gboolean displayError(gpointer data)
{
	GError *USBerror;
	USBerror = (GError *) data;
	
	gtk_entry_buffer_set_text ((GtkEntryBuffer*)priv->entrybuffer1, USBerror->message, -1);
	gtk_toggle_button_set_active((GtkToggleButton*)priv->on_off_button,
		                             FALSE);
	gtk_entry_set_icon_from_stock ((GtkEntry*)priv->entry1, GTK_ENTRY_ICON_PRIMARY, "gtk-dialog-error");

	return FALSE;
}

gboolean displayPlayIcon(gpointer data)
{
	//gtk_entry_buffer_set_text (priv->entrybuffer1, "Tracking!", -1);	
	gtk_entry_set_icon_from_stock ((GtkEntry*)priv->entry1, GTK_ENTRY_ICON_PRIMARY, "gtk-media-play");
	gtk_entry_buffer_set_text ((GtkEntryBuffer*)priv->entrybuffer1, "Device OK!", -1);
	
	return FALSE;
}

gboolean displayIdle(gpointer data)
{
	gtk_toggle_button_set_active ((GtkToggleButton*)priv->on_off_button, FALSE);
	gtk_entry_set_icon_from_stock ((GtkEntry*)priv->entry1, 
			                       GTK_ENTRY_ICON_PRIMARY, 
			                       "gtk-about");
	gtk_entry_buffer_set_text ((GtkEntryBuffer*)priv->entrybuffer1, 
	                           "Choose a USB ACM device from combo box and press activate.",
	                           -1);
	return FALSE;
}

gboolean displayDisconnect(gpointer data)
{
	gtk_toggle_button_set_active ((GtkToggleButton*)priv->on_off_button, FALSE);
	gtk_entry_set_icon_from_stock ((GtkEntry*)priv->entry1, 
			                       GTK_ENTRY_ICON_PRIMARY, 
			                       "gtk-dialog-error");
	gtk_entry_buffer_set_text ((GtkEntryBuffer*)priv->entrybuffer1, 
	                           "Device disconnected.",
	                           -1);

	return FALSE;
}

gpointer
doCancel(gpointer data)
{
	g_cancellable_cancel (mainCanceller);

	return NULL;
}

gpointer
textviewFiller(gpointer data)
{
	static GFileInputStream* USBsinkInStream;
	
	
	GError* USBerror = NULL;
	GDataInputStream* USBsinkDataInStream; 
	gsize linesize;
	gchar* linedata = NULL;
	GFile* USBsink;	

	unsigned int addr = -1, reported[5];
	char status_char = 0;
	int i, time_shift;
	float event_time[4];

	USBsink = g_file_new_for_path (gtk_combo_box_text_get_active_text((GtkComboBoxText*)priv->comboboxtext2));
	USBsinkInStream = g_file_read (USBsink, NULL, &USBerror);

	if(USBerror != NULL){
		gdk_threads_add_idle (displayError, USBerror);
		
		return NULL;
	}
	
	USBsinkDataInStream = g_data_input_stream_new ((GInputStream*)USBsinkInStream);

	gdk_threads_add_idle (displayPlayIcon, NULL);

	gdk_threads_enter ();
	gtk_text_buffer_set_text((GtkTextBuffer*)priv->textbuffer1, 
	                    	     "Start!\r\n",
		                         -1);
	gdk_threads_leave ();
	
	do{
		linedata = g_data_input_stream_read_line (USBsinkDataInStream, &linesize, mainCanceller, &USBerror);
		//g_printf("%d", linesize);
		if(!g_cancellable_is_cancelled(mainCanceller) && linedata != NULL){
			gdk_threads_enter();
			gtk_text_buffer_insert_at_cursor((GtkTextBuffer*)priv->textbuffer1, 
	                     					linedata,
		                     				linesize);

			if(g_str_has_prefix(linedata, "Sync")){
				status_char = 0;
			}
			else{
				   
				sscanf(linedata, "%4x:%8x%2x%2x%2x%2x", 
					   &addr, 
				       &reported[0], &reported[1], &reported[2], &reported[3],
				       &reported[4]);
				time_shift = g_ntohl(reported[0]);

				for(i = 0; i<4; i++){
					if(addr == addr_book[i]){
						event_time[i] = ((reported[4] < reported[2]) ? 
						                 ((int)reported[4]-(int)reported[2]) : 
							             ((int)reported[4]-(int)reported[2] - 256)) * 256; 
						event_time[i] += (float)reported[3] + (float)time_shift/2;
						
						status_char++;
						break;
					}
					else if(i == 3){
						g_printf("UNKNOWN ADDR!\n");
					}
				}

				if(status_char == 4){
					gchar buffer[G_ASCII_DTOSTR_BUF_SIZE];
					
					gtk_label_set_text ((GtkLabel*)priv->label1, 
					                    g_ascii_dtostr (buffer, G_ASCII_DTOSTR_BUF_SIZE, 
					                                    event_time[1] - event_time[0]));
					gtk_label_set_text ((GtkLabel*)priv->label2, 
					                    g_ascii_dtostr (buffer, G_ASCII_DTOSTR_BUF_SIZE, 
					                                    event_time[2] - event_time[0]));
					gtk_label_set_text ((GtkLabel*)priv->label3, 
					                    g_ascii_dtostr (buffer, G_ASCII_DTOSTR_BUF_SIZE, 
					                                    event_time[3] - event_time[0]));
					
				}
				
			}
			
			gdk_threads_leave();
			
			//g_printf("%d %X", status_char, addr);
			//g_printf("%d",time_shift);
			fflush(stdout);
		}
	}while(USBerror == NULL && linesize != 0 );

	if(USBerror != NULL){
		g_fprintf(stderr, "%s", USBerror->message);
	}

	if(linesize == 0){
		gdk_threads_add_idle (displayDisconnect,NULL);
	}
	else{
		gdk_threads_add_idle (displayIdle, NULL);
	}
	
	g_input_stream_close (USBsinkInStream, NULL, NULL);
	
	return NULL;
	
}

void
doClick (GtkWidget *widget, gpointer data)
{
	
	//static GThread* textFillerThread = NULL;
	
	if(gtk_toggle_button_get_active ((GtkToggleButton*)priv->on_off_button)){
		const GdkRGBA green = {0.0,1.0,0.0,0.2};
	
		g_printf("On!");	

		g_cancellable_reset (mainCanceller);

		gtk_widget_override_background_color(priv->textview1, 
		                                     GTK_STATE_FLAG_NORMAL,
		                                     &green);

		gtk_spinner_start ((GtkSpinner*)priv->spinner1);
		
		g_thread_new ("Text View Filler", textviewFiller, NULL);
		
	}
	else{
		const GdkRGBA red = {1.0,0.0,0.0,0.2};
		g_printf("Off!");

		gtk_text_buffer_insert_at_cursor((GtkTextBuffer*)priv->textbuffer1, 
	                     	   "*****Stop!*****\r\n",
	                     	   -1);
		gtk_widget_override_background_color(priv->textview1, 
		                                     GTK_STATE_FLAG_NORMAL,
		                                     &red);

		gtk_spinner_stop ((GtkSpinner*)priv->spinner1);
		
		g_thread_new ("Cancel", doCancel, NULL); 
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

	priv = g_malloc(sizeof(struct private));
	
	/* ANJUTA: Widgets initialization for gtk_foobar.ui - DO NOT REMOVE */
	priv->label2 = GTK_WIDGET (gtk_builder_get_object(builder, "label2"));
	priv->label3 = GTK_WIDGET (gtk_builder_get_object(builder, "label3"));
	priv->label1 = GTK_WIDGET (gtk_builder_get_object(builder, "label1"));
	priv->spinner1 = GTK_WIDGET (gtk_builder_get_object(builder, "spinner1"));
	priv->comboboxtext_entry = GTK_WIDGET (gtk_builder_get_object(builder, "comboboxtext_entry"));
	priv->comboboxtext2 = GTK_WIDGET (gtk_builder_get_object(builder, "comboboxtext2"));
	priv->entry1 = GTK_WIDGET (gtk_builder_get_object(builder, "entry1"));
	priv->entrybuffer1 = GTK_WIDGET (gtk_builder_get_object(builder, "entrybuffer1"));
	priv->on_off_button = GTK_WIDGET (gtk_builder_get_object(builder, "on_off_button"));
	priv->textview1 = GTK_WIDGET (gtk_builder_get_object(builder, "textview1"));
	priv->window = GTK_WIDGET (gtk_builder_get_object(builder, "window"));
	priv->textbuffer1 = GTK_WIDGET (gtk_builder_get_object(builder, "textbuffer1"));
	
	g_object_unref (builder);

	gtk_combo_box_set_active((GtkComboBox*)priv->comboboxtext2,0);
	gtk_spinner_stop ((GtkSpinner*)priv->spinner1);
	
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

	mainCanceller = g_cancellable_new ();

	gdk_threads_init();
	
	gdk_threads_enter();
	gtk_main ();
	gdk_threads_leave();
	
	g_free(priv);

	return 0;
}
