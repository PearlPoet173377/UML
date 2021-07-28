#include<iostream>
#include<conio.h>
#include<thread>
#include <windows.h>

using namespace std;

const unsigned int default_tank_volume = 60;
const unsigned int min_fuel_level = 5;
const unsigned int default_engine_consumption = 8;

#define Escape 27
#define Enter 13

class Tank
{
	unsigned int volume;
	double fuel_level;
	unsigned int min_level;
public:
	unsigned int get_volume()const
	{
		return volume;
	}
	double get_fuel_level()const
	{
		return fuel_level;
	}
	unsigned int get_min_level()const
	{
		return min_level;
	}
	void set_fuel_level(double fuel)
	{
		if (fuel < 0)return;
		if (this->fuel_level + fuel < volume) fuel_level += fuel;
		else fuel_level = volume;
	}
	explicit Tank(int volume) :volume(volume >= 40 && volume <= 120 ? volume : default_engine_consumption)
	{
		/*if (volume >= 40 && volume <= 120) this->volume = volume;
		else this->volume = 60;*/
		this->volume = volume >= 40 && volume <= 120 ? volume : default_tank_volume;
		fuel_level = 0;
		min_level = min_fuel_level;
		cout << "TankIsReady:\t" << this << endl;
	}
	~Tank()
	{
		cout << "TankIsOver:\t" << this << endl;
	}
	double give_fuel(double amount)
	{
		fuel_level -= amount;
		if (fuel_level < 0) fuel_level = 0;
		return fuel_level;
	}
	void info()const
	{
		cout << "Volume:\t " << volume << endl;
		cout << "Fuel:\t ";
		if (fuel_level < 10)cout << 0;
		cout << fuel_level << endl;
		cout << "MinValue:";
		if (min_level < 10)cout << 0;
		cout << min_level << endl;
	}
};

class Engine
{
	unsigned int volume;
	unsigned int power;
	double consumption;
	double consumption_per_second;

	bool started;
public:
	double get_consumption()const
	{
		return consumption;
	}
	double get_consumption_per_second()const
	{
		return consumption_per_second;
	}
	explicit Engine(double consumption)
	{
		/*if (consumption >= 4 && consumption <= 25) this->consumption = consumption;
		else consumption = 8;*/
		this->consumption = consumption >= 4 && consumption <= 25 ? consumption : default_engine_consumption;
		this->consumption_per_second = consumption * 5e-5;
		started = false;
		cout << "EngineIsReady:\t" << this << endl;
	}
	bool is_started()const
	{
		return started;
	}
	void start()
	{
		started = true;
	}
	void stop()
	{
		started = false;
	}
	~Engine()
	{
		cout << "EngineIsOver:\t" << this << endl;
	}
	void info()const
	{
		cout << "Consumption:\t" << consumption << endl;
		cout << "ConsumptionPerSec:\t" << consumption_per_second << endl;
	};
};

class Car
{
	Engine engine;
	Tank tank;
	const unsigned int MAX_SPEED;
	unsigned int speed;
	bool driver_inside;
	bool gas_pedal;
	unsigned int acceleration;
	unsigned int to_brake;

	struct Control
	{
		std::thread main_thread;
		std::thread panel_thread;
		std::thread engine_idle_thread;
		std::thread free_wheeling_thread;
	}control;
public:
	/*Car(const Engine& engine, const Tank& tank) :engine(engine), tank(tank)
	{
		cout << "CarIsReady:\t" << this << endl;
	}*/
	Car(double consumption, unsigned int volume, const unsigned int MAX_SPEED) :engine(consumption), tank(volume), MAX_SPEED(MAX_SPEED >= 90 && MAX_SPEED <= 400 ? MAX_SPEED : 250), speed(0)
	{
		driver_inside = false;
		control.main_thread = std::thread(&Car::Control_Car, this);
		cout << "CarIsReady:\t" << this << endl;
		gas_pedal = false;
		acceleration = 5;
		to_brake = 5;
	}
	~Car()
	{
		//if (control.engine_idle_thread.joinable())control.engine_idle_thread.join();
		if (control.main_thread.joinable())control.main_thread.join();
		cout << "CarIsOver:\t" << this << endl;
	}
	void get_in()
	{
		driver_inside = true;
		control.panel_thread = std::thread(&Car::panel, this);
	}
	void get_out()
	{
		driver_inside = false;
		if (control.panel_thread.joinable())control.panel_thread.join();
		system("CLS");
		cout << "You are out of car" << endl;
	}
	bool is_driver_inside()const
	{
		return driver_inside;
	}
	void fill(unsigned int fuel)
	{
		tank.set_fuel_level(fuel);
	}

	void start()
	{
		if (driver_inside && tank.get_fuel_level() > 0)
		{
			engine.start();
			control.engine_idle_thread = std::thread(&Car::engine_idle, this);
		}
	}
	void stop()
	{
		engine.stop();
		if (control.engine_idle_thread.joinable())control.engine_idle_thread.join();
	}
	void engine_idle()
	{
		using namespace std::literals::chrono_literals;
		while (engine.is_started() && tank.give_fuel(engine.get_consumption_per_second()))
		{
			std::this_thread::sleep_for(1s);
		}
		engine.stop();
	}

	void panel()const
	{
		using namespace std::literals::chrono_literals;
		while (is_driver_inside())
		{
			system("CLS");
			cout << "Engine is:\t" << (engine.is_started() ? "Started" : "Stopped") << endl;
			cout << "Fuel level:\t" << tank.get_fuel_level() << endl;
			if (tank.get_fuel_level() < tank.get_min_level())
			{
				HANDLE hOUTPUT = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hOUTPUT, FOREGROUND_RED);
				cout << "LOW FUEL" << endl;
				SetConsoleTextAttribute(hOUTPUT, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
			}
			cout << "Speed:" << speed << "\t";
			cout << "MaxSpeed: " << MAX_SPEED << endl;
			std::this_thread::sleep_for(1s);
		}
	}
	void Control_Car()
	{
		cout << "You are inside" << endl;
		using namespace std::literals::chrono_literals;
		char key;
		do
		{
			key = _getch();
			switch (key)
			{
			case 'F': case 'f':
				unsigned int fuel;
				cout << "Введите объем топлива: "; cin >> fuel; cout << endl;
				fill(fuel);
				break;
			case 'I': case 'i':
				if (is_driver_inside())
				{
					if (!engine.is_started()) start();
					else stop(); break;
				}
			case 'W': case 'w':
				if (driver_inside && engine.is_started())
				{
					accelerate();
				}
				break;
			case 'S': case 's':
				if (driver_inside && engine.is_started())
				{
					brake();
				}
			case Enter:
				if (is_driver_inside()) get_out();
				else get_in(); break;
			case Escape:
				stop();
				get_out();
				cout << "Hava La`vista baby" << endl; break;
			}
			//if (is_driver_inside())panel();
			std::this_thread::sleep_for(1ms);
			if (speed > 0 && !control.free_wheeling_thread.joinable())control.free_wheeling_thread = std::thread(&Car::free_wheeling, this);
			else if (control.free_wheeling_thread.joinable())control.free_wheeling_thread.join();
		} while (key != Escape);
	}
	void accelerate()
	{
		speed += acceleration;
		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for(1ms);
	}
	void brake()
	{
		speed += to_brake;
	}
	void free_wheeling()
	{
		using namespace std::literals::chrono_literals;
		while (speed)
		{
			speed--;
			std::this_thread::sleep_for(1s);
		}
	}
	void info()const
	{
		cout << "Engine:\t"; engine.info();
		cout << "Tank:\n"; tank.info();
		cout << "Engine is:\t" << (engine.is_started() ? "Started" : "Stopped") << endl;
		cout << "Speed:\t" << speed << "km/h\n";
		cout << "MaxSpeed:\t" << MAX_SPEED << endl;
	}
};

//#define TANK_CHECK
//#define ENGINE_CHECK

void main()
{
	setlocale(0, "Rus");
#ifdef TANK_CHECK
	Tank tank(35);
	tank.info();
	cout << "\n----------------------------\n";
	tank.set_fuel_level(30);
	tank.info();
	cout << "\n----------------------------\n";
	tank.set_fuel_level(40);
	tank.info();
#endif // TANK_CHECK
#ifdef ENGINE_CHECK
	Engine engine(10);
	engine.info();
#endif // ENGINE_CHECK

	Car car(10, 80, 220);
	//car.info();

	cout << "Press Enter to get in" << endl;
	cout << "Press F to fill the car" << endl;
}