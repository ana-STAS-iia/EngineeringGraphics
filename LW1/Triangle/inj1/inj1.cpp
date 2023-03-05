﻿#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

struct Vector3f {
    float x;
    float y;
    float z;

    Vector3f()
    {
    }

    Vector3f(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
};

GLuint VBO; //хранения дескриптора объекта буфера вершин

static void RenderSceneCB() {
    glClear(GL_COLOR_BUFFER_BIT); //очищает буферы до предустановленных значений

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //снова связываю наш буфер, готовясь к вызову отрисовки
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //вызов сообщает конвейеру, как интерпретировать данные внутри буфера

    glDrawArrays(GL_TRIANGLES, 0, 3); // рисую треугольник

    glDisableVertexAttribArray(0); //отключаю каждый атрибут вершины, т.к. он не используется сразу

    glutSwapBuffers();
}

static void InitializeGlutCallbacks() {
    glutDisplayFunc(RenderSceneCB); //обратный вызов, который отрисовывает 1 кадр
}

static void CreateVertexBuffer() {
    Vector3f Vertices[3]; //увеличила массив, чтобы он мог содержать 3 вершины
    Vertices[0] = Vector3f(-1.0f, 1.0f, 0.0f); //левая вершина
    Vertices[1] = Vector3f(0.0f, -1.0f, 0.0f); //середина
    Vertices[2] = Vector3f(1.0f, 1.0f, 0.0f); //правая вершина

    glGenBuffers(1, &VBO); //первый - количество объектов, которые вы хотите создать, а второй — адрес массива для хранения дескрипторов
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //привязать объект именованного буфера, 1 поле - атрибуты вершин
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW); //заполняем его данными, 1 поле - атрибуты вершин, 4-не собираюсь менять содержимое буфера, указываю GL_STATIC_DRAW
}

int main(int argc, char** argv) {
    glutInit(&argc, argv); //инициализирует GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); //мы настраиваем некоторые параметры GLUT
    glutInitWindowSize(500, 500); //размер окна
    glutInitWindowPosition(100, 100); //устанавливают начальное положение
    glutCreateWindow("Tutorial 3"); //заголовок

    InitializeGlutCallbacks();

    // инициализируем GLEW и проверяю наличие ошибок
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 1.0f, 0.0f, 0.0f); //устанавливаю цвет, который будет использоваться при очистке буфера кадра

    CreateVertexBuffer(); //Создает буфер вершин

    glutMainLoop();

    return 0;
}