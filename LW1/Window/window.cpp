#include <GL/freeglut.h>


static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT); //очищает буферы до предустановленных значений
    glutSwapBuffers();
}

static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB); //обратный вызов, который отрисовывает 1 кадр
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv); //инициализирую GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(500, 500); //размер окна
    glutInitWindowPosition(100, 100); //расположение окна
    glutCreateWindow("Tutorial 01"); //имя окна

    InitializeGlutCallbacks();

    glClearColor(0.0f, 0.5f, 0.0f, 0.0f); //устанавливаю цвет, который будет использоваться при очистке буфера кадра

    glutMainLoop();

    return 0;
}