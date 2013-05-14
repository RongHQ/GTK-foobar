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

#include <cairo.h>
#include <math.h>

#include <glib/gi18n.h>
#include <gio/gio.h>

struct private{

	/* ANJUTA: Widgets declaration for gtk_foobar.ui - DO NOT REMOVE */
	GtkWidget* adjustment2;
	GtkWidget* spinbutton1;
	GtkWidget* adjustment1;
	GtkWidget* entrybuffer2;
	GtkWidget* entry2;
	GtkWidget* drawingarea1;
	GtkWidget* label2;
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
float event_time[50][4];
int event_time_now=0;
int event_time_disp=0;
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

gboolean
draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	guint width, height; 
	int i, j;
	//GRand *rand1;

	gdouble a = 1.0, b, c, c_phy, x_offs, y_offs, twoa_measured[4];
	gdouble node_x[4], node_y[4]; 
	
	c_phy = g_ascii_strtod (gtk_entry_buffer_get_text 
	                    ((GtkEntryBuffer*)priv->entrybuffer2), 
	                    NULL) ;
	
	GdkRGBA color = {1.0,0.0,0.0,1.0};
	
	//rand1 = g_rand_new ();
	//color.green = g_rand_double_range (rand1, 0.0, 1.0);
	//color.blue = g_rand_double_range (rand1, 0.0, 1.0);

	width = gtk_widget_get_allocated_width (widget);
	height = gtk_widget_get_allocated_height (widget);

	//c *= (width - 200);
	//a *= (width - 200);

	c = (width - 200) / 2;
	
	x_offs = width / 2;
	y_offs = height / 2;

	node_x[0] = width - 100; node_y[0] = 100;
	node_x[1] = 100; node_y[1] = 100;
	node_x[2] = 100; node_y[2] = height - 100;
	node_x[3] = width - 100; node_y[3] = height - 100;
	
	gdk_cairo_set_source_rgba (cr, &color);
	cairo_arc (cr,
             node_x[0], node_y[0],
             10,
             0, 2 * G_PI);
	cairo_fill (cr);
	
	color.green = 0.5;
	gdk_cairo_set_source_rgba (cr, &color);

	for(j=1; j<4; j++){		
		cairo_arc (cr, node_x[j], node_y[j], 10, 0, 2 * G_PI);
		cairo_fill (cr);
	}
	

  //gtk_style_context_get_color (gtk_widget_get_style_context (widget),
  //                             0,
  //                             &color);
	color.blue = 0.5;
	color.green = 1.0;
	color.red = 0.0;
	gdk_cairo_set_source_rgba (cr, &color);

	twoa_measured[0] = event_time[event_time_disp][0] - event_time[event_time_disp][3];
	twoa_measured[1] = event_time[event_time_disp][1] - event_time[event_time_disp][2];
	twoa_measured[2] = event_time[event_time_disp][1] - event_time[event_time_disp][0];
	twoa_measured[3] = event_time[event_time_disp][2] - event_time[event_time_disp][3];

	for (j = 0; j<2 ;j++){

		a = (twoa_measured[j] / c_phy) * (width - 200);
		a /= 2;
		
		if(c > a){
			b = sqrt((c * c) - (a * a));
		}
		else{
			continue;
		}
		for(i = -node_x[j]; i <(501 - node_x[j]); i += 9){
			cairo_curve_to (cr, i + node_x[j], a * sqrt((i / b) * (i / b) + 1) + y_offs, 
				            i + node_x[j] + 3, a * sqrt(((i + 3) / b) * ((i + 3) / b) + 1) + y_offs,
				            i + node_x[j] + 6, a * sqrt(((i + 6) / b) * ((i + 6) / b) + 1) + y_offs);
		
		}
		cairo_stroke (cr);
	}

	for (j = 2; j<4 ;j++){

		a = (twoa_measured[j] / c_phy) * (width - 200);
		a /= 2;
		
		if(c > a){
			b = sqrt((c * c) - (a * a));
		}
		else{
			continue;
		}
		for(i = -node_x[j]; i <(501 - node_x[j]); i += 9){
			cairo_curve_to (cr, a * sqrt((i / b) * (i / b) + 1) + x_offs, i + node_x[j], 
				            a * sqrt(((i + 3) / b) * ((i + 3) / b) + 1) + x_offs, i + node_x[j] + 3,
				            a * sqrt(((i + 6) / b) * ((i + 6) / b) + 1) + x_offs, i + node_x[j] + 6);
		
		}
		cairo_stroke (cr);
	}
	//cairo_curve_to (cr, 10, 10, 20, 30, 30, 10);
	//cairo_curve_to (cr, 0, 0, 40, 30, 300, c);

	

 return FALSE;
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
	
	GtkTextIter iter1;
	GError* USBerror = NULL;
	GDataInputStream* USBsinkDataInStream; 
	gsize linesize;
	gchar* linedata = NULL;
	GFile* USBsink;	

	unsigned int addr = -1, reported[5];
	char status_char = 0;
	int i, time_shift;
	

	USBsink = g_file_new_for_path (gtk_combo_box_text_get_active_text((GtkComboBoxText*)priv->comboboxtext2));
	USBsinkInStream = g_file_read (USBsink, NULL, &USBerror);

	if(USBerror != NULL){
		gdk_threads_add_idle (displayError, USBerror);
		
		return NULL;
	}
	
	USBsinkDataInStream = g_data_input_stream_new ((GInputStream*)USBsinkInStream);

	gdk_threads_add_idle (displayPlayIcon, NULL);

	gdk_threads_enter ();
	gtk_text_buffer_get_end_iter ((GtkTextBuffer*)priv->textbuffer1, &iter1);
	gtk_text_buffer_insert((GtkTextBuffer*)priv->textbuffer1, 
		                   &iter1, 
	                 	   "*****Start!*****\r\n",
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
			gtk_adjustment_set_value ((GtkAdjustment*)priv->adjustment1, 
			                          gtk_adjustment_get_upper ((GtkAdjustment*)priv->adjustment1));

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
						event_time[event_time_now][i] = ((reported[4] < reported[2]) ? 
						                 ((int)reported[4]-(int)reported[2]) : 
							             ((int)reported[4]-(int)reported[2] - 256)) * 256; 
						event_time[event_time_now][i] += (float)reported[3] + (float)time_shift/2;
						event_time[event_time_now][i] /= 7575;
						event_time[event_time_now][i] *= 340;
						
						status_char++;
						break;
					}
					else if(i == 3){
						g_printf("UNKNOWN ADDR!\n");
					}
				}

				if(status_char == 4){
					//gchar buffer[G_ASCII_DTOSTR_BUF_SIZE];
					
					//gtk_label_set_text ((GtkLabel*)priv->label1, 
					//                    g_ascii_dtostr (buffer, G_ASCII_DTOSTR_BUF_SIZE, 
					//                                    event_time[1] - event_time[0]));
					//gtk_label_set_text ((GtkLabel*)priv->label2, 
					//                    g_ascii_dtostr (buffer, G_ASCII_DTOSTR_BUF_SIZE, 
					//                                    event_time[2] - event_time[0]));
					//gtk_label_set_text ((GtkLabel*)priv->label3, 
					//                    g_ascii_dtostr (buffer, G_ASCII_DTOSTR_BUF_SIZE, 
					//                                    event_time[3] - event_time[0]));
					event_time_disp = event_time_now;
					
					if(gtk_adjustment_get_upper((GtkAdjustment*)priv->adjustment2)<49){
						gtk_adjustment_set_upper((GtkAdjustment*)priv->adjustment2, event_time_now);
					}
					
					if(event_time_now == 49){
						event_time_now = 0;
					}
					else{
						event_time_now++;
					}
					gtk_adjustment_set_value((GtkAdjustment*)priv->adjustment2, event_time_disp);
					gtk_widget_queue_draw(priv->drawingarea1);
					
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

gboolean 
doPress (GtkWidget *widget, GdkEvent  *event, gpointer   user_data) 
{
	GtkTextIter iter1;
	char xytext[21];
	static gdouble x_meter, y_meter, c_phy = 2.0;
	
	if(event->type == GDK_BUTTON_PRESS){
		c_phy = g_ascii_strtod (gtk_entry_buffer_get_text 
	               		((GtkEntryBuffer*)priv->entrybuffer2), 
	           			 NULL) ;
	
		x_meter = ((((GdkEventButton*)event)->x - 255) / 310) * c_phy;
		y_meter = - ((((GdkEventButton*)event)->y - 255) / 310) * c_phy;

		g_sprintf(xytext, "x:%3.3f, y:%3.3f\n", x_meter, y_meter);
		//fflush(stdout);

		gtk_text_buffer_get_end_iter ((GtkTextBuffer*)priv->textbuffer1, &iter1);
		gtk_text_buffer_insert((GtkTextBuffer*)priv->textbuffer1, 
		                		&iter1, 
	                     		xytext,
	                     		-1);
		gtk_adjustment_set_value ((GtkAdjustment*)priv->adjustment1, 
		                          gtk_adjustment_get_upper ((GtkAdjustment*)priv->adjustment1));
	}
	else if(event->type == GDK_MOTION_NOTIFY){
		x_meter = ((((GdkEventMotion*)event)->x - 255) / 310) * c_phy;
		y_meter = - ((((GdkEventMotion*)event)->y - 255) / 310) * c_phy;

		g_sprintf(xytext, "x:%3.3f", x_meter);
		gtk_label_set_text ((GtkLabel*)priv->label1, 
							xytext);

		g_sprintf(xytext, "y:%3.3f", y_meter);
		gtk_label_set_text ((GtkLabel*)priv->label2, 
							xytext);	
	}
	
	return FALSE;
}

void
doValue (GtkSpinButton *spinbutton, gpointer user_data)
{
	if(event_time_disp != gtk_spin_button_get_value_as_int (spinbutton)){
		event_time_disp = gtk_spin_button_get_value_as_int (spinbutton);
		gtk_widget_queue_draw(priv->drawingarea1);
	}
	
}

void
doClick (GtkWidget *widget, gpointer data)
{
	GtkTextIter iter1;
	//static GThread* textFillerThread = NULL;
	//gtk_widget_queue_draw_area (priv->drawingarea1, 100, 0, 300, 300);
	if(gtk_toggle_button_get_active ((GtkToggleButton*)priv->on_off_button)){
		const GdkRGBA green = {0.0,1.0,0.0,0.2};
	
		g_printf("On!");	

		g_cancellable_reset (mainCanceller);

		gtk_widget_override_background_color(priv->textview1, 
		                                     GTK_STATE_FLAG_NORMAL,
		                                     &green);

		gtk_spinner_start ((GtkSpinner*)priv->spinner1);
		//gtk_widget_set_sensitive (priv->entry2, FALSE);
		
		g_thread_new ("Text View Filler", textviewFiller, NULL);
		
	}
	else{
		const GdkRGBA red = {1.0,0.0,0.0,0.2};
		g_printf("Off!");

		gtk_text_buffer_get_end_iter ((GtkTextBuffer*)priv->textbuffer1, &iter1);
		gtk_text_buffer_insert((GtkTextBuffer*)priv->textbuffer1, 
		                       &iter1, 
	                     	   "*****Stop!*****\r\n",
	                     	   -1);
		gtk_widget_override_background_color(priv->textview1, 
		                                     GTK_STATE_FLAG_NORMAL,
		                                     &red);

		gtk_spinner_stop ((GtkSpinner*)priv->spinner1);
		//gtk_widget_set_sensitive (priv->entry2, TRUE);
		
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
	priv->adjustment2 = GTK_WIDGET (gtk_builder_get_object(builder, "adjustment2"));
	priv->spinbutton1 = GTK_WIDGET (gtk_builder_get_object(builder, "spinbutton1"));
	priv->adjustment1 = GTK_WIDGET (gtk_builder_get_object(builder, "adjustment1"));
	priv->entrybuffer2 = GTK_WIDGET (gtk_builder_get_object(builder, "entrybuffer2"));
	priv->entry2 = GTK_WIDGET (gtk_builder_get_object(builder, "entry2"));
	priv->drawingarea1 = GTK_WIDGET (gtk_builder_get_object(builder, "drawingarea1"));
	priv->label2 = GTK_WIDGET (gtk_builder_get_object(builder, "label2"));
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
