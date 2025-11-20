#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <iostream>

// My Application Class
class MyApplication {
    public:
        GtkWidget* window;
        GtkWidget* gl_area;

        MyApplication(GtkApplication* app) {
            window = gtk_application_window_new(app);
            gtk_window_set_title(GTK_WINDOW(window), "OpenGL Area");
            gtk_window_set_default_size(GTK_WINDOW(window), 1920/2, 1080/2);


            // Create the GL Area
            gl_area = gtk_gl_area_new();

            // Connect the signals
            g_signal_connect(gl_area, "realize", G_CALLBACK(on_realize_static), this);
            g_signal_connect(gl_area, "render", G_CALLBACK(on_render_static), this);


            // Adding the GL Area to the window
            gtk_container_add(GTK_CONTAINER(window), gl_area);
            gtk_widget_show_all(window);
        }

        void on_realize() {
            gtk_gl_area_make_current(GTK_GL_AREA(gl_area));
            if (gtk_gl_area_get_error(GTK_GL_AREA(gl_area)) != NULL) {
                std::cerr << "Failed to initialize OpenGL context!" << std::endl;
                return;
            }
            std::cout << "Realize: OpenGL Context Created!" << std::endl;

            // setup VBOs and Shaders here...
        }

        // Call signal render GL context
        gboolean on_render(GdkGLContext* context) {
            // Test clear screen to red
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            return TRUE;
        }

        // Bridge Function
        static void on_realize_static(GtkGLArea* area, gpointer user_data) {
            MyApplication* self = static_cast<MyApplication*>(user_data);
            self->on_realize();
        }
        
        // Check signal if rendered
        static gboolean on_render_static(GtkGLArea* area, GdkGLContext* context, gpointer user_data) {
            MyApplication* self = static_cast<MyApplication*>(user_data);
            return self->on_render(context);
        }
};

// Activation Function
static void activate(GtkApplication* app, gpointer user_data) {

    new MyApplication(app);
}

int main(int argc, char** argv) {
    GtkApplication* app;
    int status;


    // Create the App Obj
    app = gtk_application_new("org.example.smoketest", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);


    // Run the loop
    status = g_application_run(G_APPLICATION(app), argc, argv);


    // Cleanup memory
    g_object_unref(app);

    return status;
}