#include <cerrno>
#include <iostream>
#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW\glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int wnd_width = 800, wnd_height = 600;
float left_border, right_border, top, bottom;
float x_start = 0.0f;
float y_start = 0.0f;
float z_start = 0.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 15.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void Do_Movement(float max_border, float x_screen_border, float y_screen_border);

bool keys[1024];
bool draw_net = true;
bool was_inf = false;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

const GLchar* vertex_source = "#version 460 core\n"
"layout (location = 0) in vec3 pos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"gl_Position = projection * view * model * vec4(pos, 1.0);\n"
"}\0";

const GLchar* fragment_source = "#version 460 core\n"
"out vec4 color;\n"
"uniform vec4 change_color;\n"
"void main()\n"
"{\n"
"color = change_color;\n"
"}\0";

float MathFunc(float x)
{
	return abs(sin(x)) - cos(abs(x));
}

// Функція для заповнення масиву точок відповідно до математичної функції
void InnitVertices(GLfloat* vertices, float left, float right, int num_of_points, float* up, float* down)
{
	float deltaX = (right - left) / num_of_points;

	*up = MathFunc(left);
	*down = MathFunc(left);

	for (int i = 0; i < 3 * num_of_points; i += 3)
	{
		vertices[i] = left;
		vertices[i + 1] = MathFunc(left);
		vertices[i + 2] = 0.0f;
		left += deltaX;

		if (vertices[i + 1] > *up)
		{
			*up = vertices[i + 1];
		}
		else if (vertices[i + 1] < *down)
		{
			*down = vertices[i + 1];
		}
	}
}

int main()
{
	std::cout << "Enter borders: ";
	std::cin >> left_border >> right_border;

	while (right_border - left_border <= 0.0f)
	{
		std::cout << "Enter correct borders: ";
		std::cin >> left_border >> right_border;
	}

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);	// Вказування версії OpenGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);	// Вказування версії OpenGL

	// Встановлення профайлу, для якого створюється контекст
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// Створення вікна
	GLFWwindow* window = glfwCreateWindow(wnd_width, wnd_height, "GRAPH", nullptr, nullptr);
	// Створення контексту вікна
	glfwMakeContextCurrent(window);

	// Підв'язування власних функцій callback
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Вказування розмуру вікна, що відмальовується
	glViewport(0, 0, wnd_width, wnd_height);

	glEnable(GL_DEPTH_TEST);

	// Створення шейдеру
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_source, nullptr);
	glCompileShader(vertex_shader);

	GLint success;
	GLchar infoLog[512] = {};

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex_shader, 512, nullptr, &infoLog[0]);
		std::cout << infoLog << std::endl;
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_source, nullptr);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment_shader, 512, nullptr, &infoLog[0]);
		std::cout << infoLog << std::endl;
	}

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shader_program, 512, nullptr, &infoLog[0]);
		std::cout << infoLog << std::endl;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	GLuint VAO[4] = {}, VBO[4] = {};

	// For function

	int points_quantity = 5000;
	GLfloat* func_vertices = new GLfloat[points_quantity * 3];

	InnitVertices(func_vertices, left_border, right_border, points_quantity, &top, &bottom);

	float max_border = std::max(std::max(abs(left_border), abs(right_border)), std::max(abs(top), abs(bottom)));

	glGenVertexArrays(1, &VAO[0]);
	glGenBuffers(1, &VBO[0]);

	glBindVertexArray(VAO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * points_quantity * 3, func_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)nullptr);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	delete[] func_vertices;

	// End

	// For serifs

	GLfloat serif_vertices[] =
	{
		0.0f, -0.1f, 0.0f,
		0.0f, 0.1f, 0.0f,
		-0.1f, 0.0f, 0.0f,
		0.1f, 0.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO[1]);
	glGenBuffers(1, &VBO[1]);

	glBindVertexArray(VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(serif_vertices), serif_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)nullptr);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// End

	// For lines

	glGenVertexArrays(1, &VAO[2]);
	glGenBuffers(1, &VBO[2]);

	glBindVertexArray(VAO[2]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 3, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// End

	// For arrows

	float leg = 0.0125f;
	float hypotenuse = 2 * leg;

	GLfloat arrows_vertices[] =
	{
		-hypotenuse, leg, 0.0f,
		-hypotenuse, -leg, 0.0f,
		0.0f, 0.0f, 0.0f,
		leg, -hypotenuse, 0.0f,
		-leg, -hypotenuse, 0.0f,
		0.0f, 0.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO[3]);
	glGenBuffers(1, &VBO[3]);

	glBindVertexArray(VAO[3]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(arrows_vertices), arrows_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)nullptr);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// End

	glUseProgram(shader_program);

	GLuint vertex_color_location = glGetUniformLocation(shader_program, "change_color");
	GLuint model_location = glGetUniformLocation(shader_program, "model");
	GLuint view_location = glGetUniformLocation(shader_program, "view");
	GLuint projection_location = glGetUniformLocation(shader_program, "projection");

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	constexpr float fov_y = glm::radians(45.0f);
	float aspect_ratio = (float)wnd_width / (float)wnd_height;
	float fov_x = atan(tan(fov_y / 2.0f) * aspect_ratio) * 2.0f;

	projection = glm::perspective(fov_y, aspect_ratio, 0.1f, 100.0f);
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, &projection[0][0]);

	x_start = (right_border + left_border) / 2.0f;
	y_start = (top + bottom) / 2.0f;
	z_start = ((right_border - left_border) / 2.0f) / tan(fov_x / 2.0f);

	if (z_start > 100.0f)
	{
		z_start = 99.9f;
	}
	else if (z_start < 1.0f)
	{
		z_start = 1.0f;
	}

	cameraPos.x = x_start;
	cameraPos.y = y_start;
	cameraPos.z = z_start;

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.5f);

	while (!glfwWindowShouldClose(window))
	{
		// Відлік часу створення кадру
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		float x_screen_border = cameraPos.z * tan(fov_x / 2.0f);
		float y_screen_border = cameraPos.z * tan(fov_y / 2.0f);

		// Функція контролю переміщення камери
		Do_Movement(max_border, x_screen_border, y_screen_border);

		glClearColor(0.0f / 255.0f, 33.0f / 255.0f, 74.0f / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader_program);

		model = glm::mat4(1.0f);
		view = glm::mat4(1.0f);

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(view_location, 1, GL_FALSE, &view[0][0]);

		// Drawing arrows

		glUniform4f(vertex_color_location, 4.0f / 255.0f, 187.0f / 255.0f, 236.0f / 255.0f, 1.0f);

		glBindVertexArray(VAO[3]);

		model = glm::mat4(1.0f);

		model = glm::translate(model, glm::vec3(x_screen_border + cameraPos.x, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(cameraPos.z));
		glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
		model = glm::mat4(1.0f);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		model = glm::translate(model, glm::vec3(0.0f, y_screen_border + cameraPos.y, 0.0f));
		model = glm::scale(model, glm::vec3(cameraPos.z));
		glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
		model = glm::mat4(1.0f);

		glDrawArrays(GL_TRIANGLES, 3, 6);

		glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);

		glBindVertexArray(0);

		// End

		// Drawing serifs

		glUniform4f(vertex_color_location, 4.0f / 255.0f, 187.0f / 255.0f, 236.0f / 255.0f, 1.0f);

		glBindVertexArray(VAO[1]);

		for (float i = (float)(int)(-x_screen_border + cameraPos.x); i < (float)(int)(x_screen_border + cameraPos.x) + 1.0f; i += 1.0f)
		{
			float yoffset = 0.0f;

			if (cameraPos.y > y_screen_border)
			{
				yoffset = cameraPos.y - y_screen_border;
			}
			else if (cameraPos.y < -y_screen_border)
			{
				yoffset = cameraPos.y + y_screen_border;
			}

			glm::vec3 x_direction(i, yoffset, 0.0f);
			model = glm::translate(model, x_direction);
			glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
			model = glm::mat4(1.0f);

			glDrawArrays(GL_LINES, 0, 2);

			x_direction = glm::vec3(-i, yoffset, 0.0f);
			model = glm::translate(model, x_direction);
			glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
			model = glm::mat4(1.0f);

			glDrawArrays(GL_LINES, 0, 2);
		}

		for (float i = (float)(int)(-y_screen_border + cameraPos.y); i < (float)(int)(y_screen_border + cameraPos.y) + 1.0f; i += 1.0f)
		{
			float xoffset = 0.0f;

			if (cameraPos.x > x_screen_border)
			{
				xoffset = cameraPos.x - x_screen_border;
			}
			else if (cameraPos.x < -x_screen_border)
			{
				xoffset = cameraPos.x + x_screen_border;
			}

			glm::vec3 y_direction(xoffset, i, 0.0f);
			model = glm::translate(model, y_direction);
			glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
			model = glm::mat4(1.0f);

			glDrawArrays(GL_LINES, 2, 4);

			y_direction = glm::vec3(xoffset, -i, 0.0f);
			model = glm::translate(model, y_direction);
			glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
			model = glm::mat4(1.0f);

			glDrawArrays(GL_LINES, 2, 4);
		}

		glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);

		glBindVertexArray(0);

		// End

		// Drawing function

		glUniform4f(vertex_color_location, 234.0f / 255.0f, 37.0f / 255.0f, 181.0f / 255.0f, 1.0f);

		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_LINE_STRIP, 0, points_quantity);
		glBindVertexArray(0);

		// End

		// Drawing lines

		glUniform4f(vertex_color_location, 4.0f / 255.0f, 187.0f / 255.0f, 236.0f / 255.0f, 1.0f);

		glBindVertexArray(VAO[2]);

		GLfloat line_vertices[] =
		{
			0.0f, -y_screen_border + cameraPos.y, 0.0f,
			0.0f, y_screen_border + cameraPos.y, 0.0f,
			-x_screen_border + cameraPos.x, 0.0f, 0.0f,
			x_screen_border + cameraPos.x, 0.0f, 0.0f
		};

		glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_vertices), line_vertices);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_LINES, 0, 4);

		glBindVertexArray(0);

		// End

		// Draw net

		if (draw_net)
		{
			glUniform4f(vertex_color_location, 4.0f / 255.0f, 187.0f / 255.0f, 236.0f / 255.0f, 0.2f);

			glBindVertexArray(VAO[2]);

			for (float i = (float)(int)(-x_screen_border + cameraPos.x); i < (float)(int)(x_screen_border + cameraPos.x) + 1.0f; i += 1.0f)
			{
				glm::vec3 x_direction(i, 0.0f, 0.0f);

				x_direction = glm::vec3(i, 0.0f, 0.0f);
				model = glm::translate(model, x_direction);
				glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
				model = glm::mat4(1.0f);

				glDrawArrays(GL_LINES, 0, 2);

				x_direction = glm::vec3(-i, 0.0f, 0.0f);
				model = glm::translate(model, x_direction);
				glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
				model = glm::mat4(1.0f);

				glDrawArrays(GL_LINES, 0, 2);
			}

			for (float i = (float)(int)(-y_screen_border + cameraPos.y); i < (float)(int)(y_screen_border + cameraPos.y) + 1.0f; i += 1.0f)
			{
				glm::vec3 y_direction(0.0f, i, 0.0f);

				y_direction = glm::vec3(0.0f, i, 0.0f);
				model = glm::translate(model, y_direction);
				glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
				model = glm::mat4(1.0f);

				glDrawArrays(GL_LINES, 2, 4);

				y_direction = glm::vec3(0.0f, -i, 0.0f);
				model = glm::translate(model, y_direction);
				glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);
				model = glm::mat4(1.0f);

				glDrawArrays(GL_LINES, 2, 4);
			}

			glUniformMatrix4fv(model_location, 1, GL_FALSE, &model[0][0]);

			glBindVertexArray(0);
		}

		// End

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, VAO);
	glDeleteBuffers(1, VBO);

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

void Do_Movement(float max_border, float x_screen_border, float y_screen_border)
{
	GLfloat cameraSpeed = 5.0f * deltaTime;

	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		cameraPos += cameraSpeed * cameraUp;
	}
	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		cameraPos -= cameraSpeed * cameraUp;
	}
	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (keys[GLFW_KEY_Q] || keys[GLFW_KEY_KP_ADD])
	{
		cameraPos += cameraSpeed * cameraFront;
	}
	if (keys[GLFW_KEY_E] || keys[GLFW_KEY_KP_SUBTRACT])
	{
		cameraPos -= cameraSpeed * cameraFront;
	}

	max_border *= 3.0f;

	if (cameraPos.y + y_screen_border > max_border)
	{
		cameraPos.y = max_border - y_screen_border;
	}
	else if (cameraPos.y - y_screen_border < -max_border)
	{
		cameraPos.y = -max_border + y_screen_border;
	}

	if (cameraPos.x + x_screen_border > max_border)
	{
		cameraPos.x = max_border - x_screen_border;
	}
	else if (cameraPos.x - x_screen_border < -max_border)
	{
		cameraPos.x = -max_border + x_screen_border;
	}

	if (x_screen_border > max_border || y_screen_border > max_border)
	{
		cameraPos += cameraSpeed * cameraFront;
	}
	else if (cameraPos.z < 1.0f)
	{
		cameraPos.z = 1.0f;
	}
	else if (cameraPos.z > 100.0f)
	{
		cameraPos.z = 99.9f;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		draw_net = !draw_net;
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		cameraPos.z = 3.0f;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		cameraPos.z = z_start;
	}
	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		cameraPos = glm::vec3(x_start, y_start, z_start);
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		cameraPos.x = x_start;
		cameraPos.y = y_start;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	wnd_width = width;
	wnd_height = height;
}