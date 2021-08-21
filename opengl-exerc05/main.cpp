
#include <iostream>
#include <string>
#include <vector>

#include <Eigen/Dense>

#include <GL/glew.h>	//GLEW
#include <GLFW/glfw3.h> //GLFW

#include "Scene.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

using namespace std;
using namespace Eigen;

#define ERROR()                                                                                   \
	{                                                                                             \
		std::set_terminate([]() {                                                                 \
			std::cout << "Erro na linha " << __LINE__ << " do arquivo " << __FILE__ << std::endl; \
			std::abort();                                                                         \
		});                                                                                       \
		std::terminate();                                                                         \
	}

// GLSL 3.3 -> MAJOR.MINOR
#define GLSL_MAJOR_VERSION 3
#define GLSL_MINOR_VERSION 3

//funções
GLFWwindow *createWindow(int winWidth, int winHeight, string winTitle, bool fullscreen = false);
void destroyWindow(GLFWwindow *win);

//evento de teclado (o evento é disparado cada vez que usuário pressiona uma tecla)
//ideal para ativar ou desavivar uma funcionalidade do sistema via teclado
void keyEvent(GLFWwindow *win, int key, int scancode, int action, int mods);

//evento de teclado (o evento é disparado a cada passada do loop de renderização)
//ideal para controlar a movimentação da câmera
void renderKeyEvent(GLFWwindow *win);

//GUI
void initImGUI(GLFWwindow *win);
void destroyGUI();
void renderGUI(GLFWwindow *win);

int main(void)
{

	Scene scene;
	GLFWwindow *window = nullptr;

	//--------JANELA--------
	window = createWindow(1024, 768, "Exercício 5"); //inicializa as bibliotecas GLEW e GLFW e cria a janela

	//--------SHADERS--------
	GLuint prog = Shader::createProgram("shaders/shader.vs", "shaders/shader.fs"); //compila os shaders, gerando um programa
	if (!prog)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		ERROR(); //termina a execução do programa
	}
	scene.addProgram(prog); //adiciona o programa na cena. Uma cena pode ter múltiplos programas...

	//--------OBJETOS--------
	vector<Vector3f> vertexPos; //coordenadas dos vértices de um objeto (um objeto pode ter mais de um triângulo)
	vertexPos.push_back(Vector3f(1.0f, 1.0f, 0.0f));
	vertexPos.push_back(Vector3f(0.0f, 0.0f, 0.0f));
	vertexPos.push_back(Vector3f(1.0f, -1.0f, 0.0f));

	Object triangle1;
	triangle1.cor = Vector3f(1.0f, 0.0f, 0.50f); //Rosa
	triangle1.create(vertexPos); //cria o objeto com base na opsição dos vértices
	scene.addObject(triangle1);	 //adiciona o objeto na cena. Uma cena pode ter múltiplos objetos...

	vertexPos.clear();
	vertexPos.push_back(Vector3f(-1.0f, 1.0f, 0.0f));
	vertexPos.push_back(Vector3f(0.0f, 0.0f, 0.0f));
	vertexPos.push_back(Vector3f(-1.0f, -1.0f, 0.0f));

	Object triangle2;
	triangle2.cor = Vector3f(0.50f, 1.0f, 0.0f); //Verde
	triangle2.create(vertexPos);
	scene.addObject(triangle2);

	//--------EVENTOS--------

	//--------REFERENCIA GLOBAL DA CENA--------
	//Passar o ponteiro da cena para a biblioteca GLFW.
	//Assim, pode-se ter acesso a cena criada em qualquer lugar do código, usando o método "glfwGetWindowUserPointer".
	//Este recurso é útil quando ocorre um evento de teclado ou mouse e precisamos ter acesso a um dado da cena
	glfwSetWindowUserPointer(window, &scene);

	//--------CONFIGURACOES DO OPENGL--------
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //cor de fundo da tela - > RGBA

	//ativar o teste de profundidade para cada fragmento processado dentro do fragment shader
	//antes de cada fragmento ser desenhado é realizada uma comparação com o valor de profundidade
	glEnable(GL_DEPTH_TEST);

	//define qual algoritmo sera usado para comparar os valores de profundidade dentro do fragment shader
	//https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDepthFunc.xhtml
	//glDepthFunc(GL_LEQUAL);
	glDepthFunc(GL_LESS);

	//--------INICIALIZAR GUI--------
	initImGUI(window);

	//--------LOOP DE RENDERIZACAO--------
	while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) //a cada iteração, um quadro é renderizado!
	{

		//Limpa os buffers que armazenam os valores de pronfundidade de cada fragmento para seu valor inicial (1.0)
		//Caso isso não seja feito, os fragmentos de novos triângulos serão comparados com os fragmentos renderizados do quadro anterior!
		glClear(GL_DEPTH_BUFFER_BIT);

		//Limpa os buffers que armazenam os valores de cor
		glClear(GL_COLOR_BUFFER_BIT);

		//especifica o uso dos shaders previamente compilados (vertex shader & fragment shader)
		//os shaders compilados foram anexados a um ponteiro denominado programa
		GLuint prog = scene.programs[0];
		glUseProgram(prog);

		for (size_t i = 0; i < scene.objects.size(); i++)
		{
			//Atribuindo o valor da cor para o FS
			GLuint uniformRef = glGetUniformLocation(prog, "colorIn");
			glUniform3fv(uniformRef, 1, scene.objects[i].cor.data());
			scene.objects[i].render();
		}
		//Inativando a função pra alterar a cor pq sim
		//renderKeyEvent(window);

		//processa os eventos de teclado e mouse
		glfwPollEvents();
		renderGUI(window);

		//atualiza a janela de visualização com o quadro renderizado
		glfwSwapBuffers(window);

	} //fim while renderizacao

	//--------DESALOCA MEMORIA--------
	destroyGUI();
	scene.destroy();
	destroyWindow(window);

	return EXIT_SUCCESS;

} //fim main

void renderKeyEvent(GLFWwindow *win)
{
	Scene *scene = reinterpret_cast<Scene *>(glfwGetWindowUserPointer(win));

	if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
	{

		scene->objects[0].cor = Vector3f(0.5f, 0.5f, 0.5f);
	}

	if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
	{

		scene->objects[0].cor = Vector3f(0.5f, 0.0f, 0.5f);
	}
}

void destroyWindow(GLFWwindow *win)
{
	//remove a janela
	glfwDestroyWindow(win);
	glfwTerminate();
}

GLFWwindow *createWindow(int winWidth, int winHeight, string winTitle, bool fullscreen)
{

	GLFWwindow *window = nullptr;

	//inicializar  a biblioteca GLFW
	if (!glfwInit())
		ERROR(); //termina a execução do programa

	//define que a versão minímina necessária do OPENGL é 3.3 ou superior!
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLSL_MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLSL_MINOR_VERSION);

	//não permite redimensionar a janela
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	if (fullscreen)
	{
		const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		//pega informacoes do monitor
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		//cout << mode->redBits << endl;
		//cout << mode->greenBits << endl;
		//cout << mode->blueBits << endl;
		//cout << mode->refreshRate << endl << endl;

		//cria a janela
		window = glfwCreateWindow(mode->width, mode->height, winTitle.c_str(), glfwGetPrimaryMonitor(), NULL); //tela cheia
	}
	else
	{
		window = glfwCreateWindow(winWidth, winHeight, winTitle.c_str(), NULL, NULL); //janela
	}

	//veririca se a janela foi criada com sucesso
	if (!window)
	{
		glfwTerminate();
		ERROR(); //termina a execução do programa
	}

	//cria o contexto de renderização
	glfwMakeContextCurrent(window);

	//iniciliza a biblioteca GleW
	if (glewInit() != GLEW_OK)
		ERROR(); //termina a execução do programa

	//VSYNC
	//Monitor de 240HZ -> 0= DESATIVADO, 1= 240FPS, 2=120FPS, 3=80FPS
	glfwSwapInterval(0);

	return window;

} // fim createWindow
// ----------- FUNÇÕES GUI ---------------
void initImGUI(GLFWwindow *win)
{

	//iniciar IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// cor da janela
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	//jenela de renderização
	ImGui_ImplGlfw_InitForOpenGL(win, true);

	//versão do GLSL -> 3.3
	std::string glslVersion = Shader::shaderProcessorVersion(GLSL_MAJOR_VERSION, GLSL_MINOR_VERSION);
	ImGui_ImplOpenGL3_Init(glslVersion.c_str());
}

void destroyGUI()
{

	if (!ImGui::GetCurrentContext())
		return;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
void renderGUI(GLFWwindow *win)
{

	//verifica se imGUI foi inicializada
	if (!ImGui::GetCurrentContext())
		return;

	//pega instância da cena
	Scene* scene = reinterpret_cast<Scene*>(glfwGetWindowUserPointer(win));

	
	//PASSO 1 - Cria-se um frame -> ÚNICO FRAME (mesmo que varias janelas sejam criadas)
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame(); //novo frame

	//PASSO 2 - opcoes de configuração da GUI
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
	window_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;



	//PASSO 3 - Criar a GUI
	//criar uma janela, pode-se criar mais que uma...
	ImGui::Begin("Seletor de cores", nullptr, window_flags);
	{
		ImGui::SetWindowPos(ImVec2(0, 0)); //posicao

		ImGui::Text("\n");
		ImGui::Separator();
		ImGui::Text("Corzitas\n");
		ImGui::ColorEdit3("Triangulo Direito", scene->objects[0].cor.data());

		ImGui::Separator();
		ImGui::ColorEdit3("Triangulo Esquerdo", scene->objects[1].cor.data());


	}
	ImGui::End();

	//PASSO 4 - renderizar GUI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

} //fim renderGUI