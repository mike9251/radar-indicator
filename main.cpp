﻿#include <freeglut.h>// заголовочный файл библиотеки для работы с окнами и устройствами ввода
#include "radar.h"// заголовочный файл с реализацией индикатора радара

using namespace radarIndicator; // используем пространство имен, определенное в radar.h

void timer(int value); //ф-ия обработчик glutTimerFunc, вызывается по прошествию установленного времени
void update(); //используем данную ф-ию для обновления угла обзора
void keyboard(unsigned char key, int, int);// ф-ия обработчик glutKeyboardFunc, вызывается после ввода данных с клавиатуры

int main(int argc, char ** argv)
{
	//инициализация переменных состояния GLUT и начало сессии с системой управления окнами
	glutInit(&argc, argv);
	//устанавливаем режим состояния экрана. Используем окно с двойным буфером, цветовое пространство RGBA
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	//задаем размеры рабочего окна
	glutInitWindowSize(width, height);
	//задаем расположение окна
	glutInitWindowPosition(100, 100);
	//создаем окно
	glutCreateWindow("radar");
	//вызываем ф-ию инициализации. Очишаем окно и задаем параметры целей
	Initialize();
	//устанавливаем функцию рисования для текущего окна. ф-ия display будет вызывать OpenGL инструкции для отрисовки
	glutDisplayFunc(display);
	//устанавливаем функцию обратного вызова, вызываемую GLUT при нажатии одной из клавиш
	glutKeyboardFunc(keyboard);
	//регистрирует функцию обратного вызова, которую нужно вызывать по прошествии msecs миллисекунд 
	glutTimerFunc(0, timer, 0);
	//основной цикл GLUT обработки событий. В цикле событий обрабатываются все сообщения клавиатуры, мыши, таймера, перерисовывания и другие сообщения окна
	glutMainLoop();

	return 0;
}

void update()
{
	if (angle != 360)
		angle += 1;
	else
		angle = 0;
}

void keyboard(unsigned char key, int q, int s)
{
	if (key == 27)	// если нажата клавиша Esc, тогда завершаем работу
		exit(0);
}

void timer(int value) 
{
	update();			// обновляем значение угла
	glutPostRedisplay();  // перерисовываем окно
	glutTimerFunc(17, timer, 0); // устанавливаем следующий таймер. Значение таймера устанавливаем 17мсек - столько времени уходит на поворот луча на 1 градус, т.к. задано, что луч вращается со скоростью 10 оборотов/мин
}