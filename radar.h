#include <freeglut.h>
#include <math.h> // заголовочный файл с библиотеки математических ф-ий
#include <vector> // для использования векторов

using namespace std; // используем пространство имен std

namespace radarIndicator // открываем область пространства имен radarIndicator
{
	// размеры рабочего окна
	int width = 600;
	int height = 600;

	//Максимальный радиус 150 км
	int maxDistance = 150;

	//используем для масштабирования
	int scalePixel = 2;

	//Текущий угол радара
	int angle = 0;

	template <typename T>// объявляем структуру данных для хранения удобного хранения координат
	struct Vector2
	{
		T x, y;

		Vector2(T x, T y) : x(x), y(y)
		{}
	};

	struct Targets// объявляем структуру для описания параметров цели
	{
		int r;	// расстояние от центра, на котором находится цель [км]
		float v;	// скорость движения цели [км/ч]
		int alpha;// азимут движения цели [градусы]
		bool hide;  // эту переменную используем для того, чтобы стирать отметку цели передним фронтом луча и наносить обновленное значение задним фронтом луча

		Targets(float r, float v, int alpha, bool hide) : r(r), v(v), alpha(alpha), hide(hide)
		{}
	};

	// ф-ия для обновления расстояния до цели
	void updateTargets(Targets &targets); 

	// для хранения параметров целей используем вектор, содержащий элементы с типом данных Targets
	vector<Targets> targets;

	// объявляем структуру для удобной работы с цветовым пространством RGBA
	struct Color
	{
		float r;
		float g;
		float b;
		float a;

		Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a)
		{}
	};

	// объявляем класс, в котором реализуем отрисовку индикатора радара
	class Circle
	{
		// центр индикатора радара
		Vector2<int> position;
		// угол, в пределах которого будем выполнять отрисовку
		Vector2<float> angle;
		// радиус индикатора радара
		int radius;
		// переменная для хранения значения цвета заливки
		Color color;

	public:
		// конструктор класса Circle, производим инициализацию переменных состояния класса
		Circle(Vector2<int> position, Vector2<float> angle, int r, Color color) : position(position), angle(angle), radius(r), color(color)
		{}

		//отрисовка круга и сектора обзора
		void Draw(Vector2<int>, Vector2<float>, int, int, bool);

		//отрисовка точки (объекта) на радаре
		void DrawPoint(Vector2<float>, Color, float, int, int, int);

		//отрисовка объектов
		void DrawObject();

		//отрисовка сетки по дальности
		void DrawCircle(Vector2<float>, int);

		//отрисовка азимутальных линий
		void DrawLine(float angle);
	};

	void Circle::Draw(Vector2<int> position, Vector2<float> angle, int step, int r, bool beam)
	{
		if (beam == true)
		{
			// включаем блендинг, чтобы сделать луч прозрачным
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);
		}
		// задаем яркость для отрисовки объекта
		glColor4f(color.r, color.g, color.b, color.a);
		//задаем примитив GL_TRIANGLE_FAN - каждый треугольник задается первой вершиной и последующими парами (т.е. треугольники строятся вокруг первой вершины)
		glBegin(GL_TRIANGLE_FAN); 
		// задаем первую вершину
		glVertex2f(position.x, position.y);
		// в цикле отрисовываем фигуру в заданном угле  angle.x - начало, angle.y - конец. Движемся против часовой стрелки с шагом step
		for (int i = angle.x; i <= angle.y; i += step) 
		{
			// переводим градусы в радианы
			float alpha_rad = (float)i / 57.2974;
			// задаем следующую вершину
			// производим преобразование полярных координат в декартовые
			glVertex2f(cos(alpha_rad) * r, sin(alpha_rad) * r + position.y);
			// просматриваем все цели
			for(int j = 0; j < targets.size(); ++j)
			{
				//если передний фронт луча совпадаетс направление движения цели
				if (angle.y == targets[j].alpha && beam == true)
				{
					//тогда не отображаем цель на радаре
					targets[j].hide = true;
				}
				//если задний фронт луча совпадает с направлением движения цели и луч полностью отрисован
				if (angle.x == targets[j].alpha && i == angle.y && beam == 1)
				{
					// тогда разрешаем отрисовку цели
					targets[j].hide = false;
					// обновляем расстояние до цели
					updateTargets(targets[j]);
				}
			}
		}
		//завершаем отрисовку
		glEnd();
		if (beam == true)
		{
			// отключаем блендинг
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
		}
	}

	void Circle::DrawPoint(Vector2<float> angle, Color color, float rad, int distance, int step, int r)
	{
		// angle - от 0 до 360 градусов, т.к. рисуем круглую точку
		// rad - угол движения цели в радианах
		// distance - расстояние до цели
		// step - шаг, с которым будем отрисовывать круглую точку
		// r - радиус точки

		// задаем яркость для отрисовки объекта
		glColor3f(color.r, color.g, color.b);
		//задаем примитив для отрисовки GL_TRIANGLE_FAN
		glBegin(GL_TRIANGLE_FAN);
		// задаем первую вершину
		glVertex2f(cos(rad) * (distance * scalePixel), sin(rad) * (distance * scalePixel) + this->position.y);
		// рисуем круг с радиусом r вокруг цели
		for (int i = angle.x; i <= angle.y; i += step)
		{
			float a = (float)i / 57.2974;
			glVertex2f(cos(a) * r + cos(rad) * (distance * scalePixel), sin(a) * r + sin(rad) * (distance * scalePixel) + this->position.y);
		}
		glEnd();
	}

	// подготавливаем отрисовку цели
	void Circle::DrawObject()
	{
		// проходим по всем целям
		for(int i = 0; i < targets.size(); ++i){
			// переводим градусы в радианы
			float alpha_rad = targets[i].alpha / 57.2974;
			// проверяем, если расстояние до цели больше maxDistance или достигло 0
			if (targets[i].r > maxDistance || targets[i].r <= 0)
			{
				// тогда устанавливаем значение равное maxDistance
				targets[i].r = maxDistance;
			}
			// если разрешено отображать цель
			if(targets[i].hide == false)
				// тогда запускаем метод DrawPoint
				// step = 30 градусов, чтобы нарисовать круглую точку, будет использоваться 12 вершин
				this->DrawPoint(Vector2<float>(0, 360), Color(1, 0, 0, 1), alpha_rad, targets[i].r, 30, 5);
		}
	}

	// метод для отрисовки сетки по дальности
	void Circle::DrawCircle(Vector2<float> angle, int r)
	{
		glColor3f(color.r, color.g, color.b);
		//каждая пара вершин задает линию (т.е. конец предыдущей линии является началом следующей). последняя вершина соединяется с первой и получается замкнутая фигура
		glBegin(GL_LINE_LOOP); 
		// рисуем окружность
		for (int i = angle.x; i <= angle.y; i++) {
			float alpha_rad = (float)i / 57.2974; //Приводим к радианам
			glVertex2f(cos(alpha_rad) * r, sin(alpha_rad) * r + this->position.y);
		}
		glEnd();
	}

	// метод для отрисовки азимутальных линий
	void Circle::DrawLine(float angle)
	{
		glColor3f(color.r, color.g, color.b);
		// каждая отдельная пара вершин задает линию
		glBegin(GL_LINES);
		// первая вершина линии, r = 0
		glVertex2f(this->position.x, this->position.y); 
		float alpha_rad = angle / 57.2974;
		//вторая вершина, r = 150km
		glVertex2f(cos(alpha_rad) * radius, sin(alpha_rad) * radius + this->position.y);
		glEnd();
	}

	void Initialize()
	{
		// устанавливаем черный цвет для заливки окна
		glClearColor(0, 0, 0, 1);
		// загружаем матрицу проекции
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		// устанавливаем ортогональную проекцию
		gluOrtho2D(-width / 2, width / 2, -height / 2, height / 2);
		// загружаем модельно-видовую матрицу
		glMatrixMode(GL_MODELVIEW);

		// создадим несколько целей
		targets.push_back(Targets(150, 1200, 35, false));
		targets.push_back(Targets(150, 1200, 75, false));
		targets.push_back(Targets(150, 1200, 90, false));
		targets.push_back(Targets(150, 1200, 210, false));
		targets.push_back(Targets(150, 1200, 125, false));
		targets.push_back(Targets(150, 1200, 170, false));
		targets.push_back(Targets(150, 1200, 310, false));
		targets.push_back(Targets(150, 1200, 260, false));
	}

	// ф-ия для обновления расстояния до цели
	void updateTargets(Targets &targets)
	{
		// считаем, что луч сделал полный оборот, следовательно смещение цели определяется скоростью v и временем в часах, необходимым для смещения луча на 360 градусов
		targets.r -= targets.v * 0.00167;
	}

	void display()
	{
		// заливаем все окно черным цветом
		glClear(GL_COLOR_BUFFER_BIT);

		//фон радара
		Circle Back(Vector2<int>(0, 0), Vector2<float>(0, 360), height / 2, Color(0, 0.8, 0, 1));
		Back.Draw(Vector2<int>(0, 0), Vector2<float>(0, 360), 5, height / 2, false);

		//рисуем разметку радара по дальности
		Circle Marking(Vector2<int>(0, 0), Vector2<float>(0, 360), height / 2, Color(0.9, 0.9, 0.9, 1));
		for (float i = 1; i >= 0.06f; i -= 0.067f)
			Marking.DrawCircle(Vector2<float>(0, 360), height / 2 * i);

		//рисуем азимутальные линии
		for (float i = 0; i <= 360; i += 30.f)
			Marking.DrawLine(i);

		//луч радара
		Circle Beam(Vector2<int>(0, 0), Vector2<float>(angle, angle + 5), height / 2, Color(1, 1, 1, 0.7));
		Beam.Draw(Vector2<int>(0, 0), Vector2<float>(angle, angle + 5), 1, height / 2, true);

		// отображение целей
		Back.DrawObject();
		// после отрисовки переставляем буфера
		glutSwapBuffers();
	}
}
