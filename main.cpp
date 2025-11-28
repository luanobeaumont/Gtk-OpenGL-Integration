#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <iostream>
#include <vector>

// --- SHADER SOURCE CODE ---
// The Vertex Shader: Takes the raw points and puts them in position
const char* vertex_shader_src = R"(
#version 330 core
layout(location = 0) in vec2 position; // We are sending 2 floats (X, Y)

void main() {
    // OpenGL expects a vec4 (X, Y, Z, W). We set Z to 0.0 and W to 1.0.
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}
)";

// The Fragment Shader: Colors the pixels inside the triangle
const char* fragment_shader_src = R"(
#version 330 core
out vec4 color;

void main() {
    // RGBA: 1.0 Red, 0.5 Green, 0.2 Blue (An orange-ish color)
    color = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

class IsoEngine {
public:
    GtkWidget* window;
    GtkWidget* gl_area;

    // OpenGL Handles (IDs)
    GLuint program_id; // The compiled shader program
    GLuint vao_id;     // The Vertex Array Object (The settings wrapper)
    GLuint vbo_id;     // The Vertex Buffer Object (The raw data)

    IsoEngine(GtkApplication* app) {
        window = gtk_application_window_new(app);
        gtk_window_set_title(GTK_WINDOW(window), "IsoEngine - Pre-Alpha");
        gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

        gl_area = gtk_gl_area_new();
        
        // Connect Signals
        g_signal_connect(gl_area, "realize", G_CALLBACK(on_realize_static), this);
        g_signal_connect(gl_area, "render", G_CALLBACK(on_render_static), this);
        g_signal_connect(gl_area, "unrealize", G_CALLBACK(on_unrealize_static), this);

        gtk_container_add(GTK_CONTAINER(window), gl_area);
        gtk_widget_show_all(window);
    }

    // --- HELPER: Compile a shader ---
    GLuint create_shader(GLenum type, const char* src) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);

        // Check for errors
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "SHADER COMPILATION ERROR:\n" << infoLog << std::endl;
        }
        return shader;
    }

    // --- 1. REALIZE: Setup GPU Memory ---
    void on_realize() {
        gtk_gl_area_make_current(GTK_GL_AREA(gl_area));
        if (gtk_gl_area_get_error(GTK_GL_AREA(gl_area)) != NULL) return;

        // A. Build Shaders
        GLuint v_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_src);
        GLuint f_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

        program_id = glCreateProgram();
        glAttachShader(program_id, v_shader);
        glAttachShader(program_id, f_shader);
        glLinkProgram(program_id);

        // Cleanup individual shaders (linked into program now)
        glDeleteShader(v_shader);
        glDeleteShader(f_shader);

        // B. Define Data (YOUR TRIANGLE)
        float triangle_data[] = {
             0.0f,  0.5f,
             0.5f, -0.5f,
            -0.5f, -0.5f
        };

        // C. Create Buffers
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(1, &vbo_id);

        // D. Upload Data
        glBindVertexArray(vao_id); // 1. Bind VAO first!
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id); // 2. Bind VBO
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_data), triangle_data, GL_STATIC_DRAW); // 3. Upload

        // E. Teach OpenGL how to read the data
        // "Attribute 0 has 2 floats (vec2), is not normalized, stride is 2*float size, start at 0"
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        std::cout << "Engine Online: GPU Ready." << std::endl;
    }

    // --- 2. RENDER: Draw Frame ---
    gboolean on_render(GdkGLContext* context) {
        // Clear background to Dark Grey (SimCity void color)
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw Triangle
        glUseProgram(program_id);
        glBindVertexArray(vao_id);
        glDrawArrays(GL_TRIANGLES, 0, 3); // Draw 3 vertices

        return TRUE;
    }

    // --- 3. UNREALIZE: Cleanup ---
    void on_unrealize() {
        gtk_gl_area_make_current(GTK_GL_AREA(gl_area));
        glDeleteBuffers(1, &vbo_id);
        glDeleteVertexArrays(1, &vao_id);
        glDeleteProgram(program_id);
        std::cout << "Engine Shutdown." << std::endl;
    }

    // --- STATICS ---
    static void on_realize_static(GtkGLArea* area, gpointer user_data) {
        ((IsoEngine*)user_data)->on_realize();
    }
    static gboolean on_render_static(GtkGLArea* area, GdkGLContext* context, gpointer user_data) {
        return ((IsoEngine*)user_data)->on_render(context);
    }
    static void on_unrealize_static(GtkGLArea* area, gpointer user_data) {
        ((IsoEngine*)user_data)->on_unrealize();
    }
};

static void activate(GtkApplication* app, gpointer user_data) {
    new IsoEngine(app);
}

int main(int argc, char** argv) {
    GtkApplication* app = gtk_application_new("org.iso.engine", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}