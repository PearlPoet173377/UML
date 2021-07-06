#include<iostream>
#include<conio.h>
#include<thread>
using namespace std;

#define Escape	27
#define Enter	13

const unsigned int DEFAULT_TANK_VOLUME = 60;
const unsigned int MIN_FUEL_LEVEL = 5;
const unsigned int DEFAULT_ENGINE_CONSUMPTION = 8;

class Tank
{
	const unsigned int VOLUME;
	double fuel_level;
	unsigned int min_level;

public:
	
	unsigned int get_volume()const
	{
		return VOLUME;
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
		if (this->fuel_level + fuel < VOLUME)
			fuel_level += fuel;
		else
			fuel_level = VOLUME;
	}

	explicit Tank(int volume) :VOLUME(volume >= 40 && volume <= 120 ? volume : DEFAULT_TANK_VOLUME)
	{
		/*if (volume >= 40 && volume <= 120)
			this->volume = volume;
		else
			this->volume = 60;*/
			//this->VOLUME = volume >= 40 && volume <= 120 ? volume : DEFAULT_TANK_VOLUME;
		fuel_level = 0;
		min_level = MIN_FUEL_LEVEL;
		cout << "TankReady:\t" << this << endl;
	}

	~Tank()
	{
		cout << "TankIsOver:\t" << this << endl;
	}

	double give_fuel(double amount)
	{
		fuel_level -= amount;
		if (fuel_level < 0)fuel_level = 0;
		return fuel_level;
	}

	void info()const
	{
		cout << "Volume:\t" << VOLUME << endl;
		cout << "Fuel:\t";
		if (fuel_level < 10)cout << 0;
		cout << fuel_level << endl;
		cout << "MinFuel:";
		if (min_level < 10)cout << 0;
		cout << min_level << endl;
	}

};

class Engine
{
	//Engine properties:
	unsigned int volume;
	unsigned int power;
	double consumption;	//liters per 100 km
	double consumption_per_second;	//liters per second

	//Engine state:
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
		/*if (consumption >= 4 && consumption <= 25)
			this->consumption = consumption;
		else
			consumption = 8;*/
		this->consumption = consumption >= 4 && consumption <= 25 ? consumption : DEFAULT_ENGINE_CONSUMPTION;
		this->consumption_per_second = consumption * 5e-5; //consumption * 5*10^-4
		started = false;
		cout << "EngineReady:\t" << this << endl;
	}

	~Engine()
	{
		cout << "EngineIsOver:\t" << this << endl;
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

	void info()const
	{
		cout << "Consumption:\t\t" << consumption << endl;
		cout << "ConsumptionPerSec:\t" << consumption_per_second << endl;
	}

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
	Car(double consumption, unsigned int volume, const unsigned int MAX_SPEED) :
		engine(consumption), tank(volume), MAX_SPEED(MAX_SPEED >= 90 && MAX_SPEED <= 400 ? MAX_SPEED : 250), speed(0)
	{
		//this->engine = consumption;
		//this->MAX_SPEED = MAX_SPEED;
		driver_inside = false;
		gas_pedal = false;
		acceleration = 5;
		control.main_thread = std::thread(&Car::control_car, this);
		cout << "CarIsReady:\t" << this << endl;
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
		while (engine.is_started() && tank.give_fuel(engine.get_consumption_per_second() * 100))
			std::this_thread::sleep_for(1s);
		engine.stop();
	}

	void panel()const
	{
		using namespace std::literals::chrono_literals;
		while (driver_inside)
		{
			system("CLS");
			cout << "Engine is " << (engine.is_started() ? "started" : "stopped") << endl;
			cout << "Fuel level:\t" << tank.get_fuel_level() << "\t";
			if (tank.get_fuel_level() < tank.get_min_level()) cout << "LOW FUEL";
			cout << endl;
			cout << "Speed:    " << speed << "\t";
			cout << "MaxSpeed: " << MAX_SPEED << endl;
			std::this_thread::sleep_for(0.5s);
		}
	}

	void control_car()
	{
		//cout << "You are inside" << endl;
		using namespace std::literals::chrono_literals;
		char key;
		do
		{
			key = _getch();
			switch (key)
			{
			case 'F':
			case 'f':
				unsigned int fuel;
				cout << "Введите объем топлива: "; cin >> fuel; cout << endl;
				fill(fuel);
				break;
			case 'I':
			case 'i':if (driver_inside)
			{
				if (!engine.is_started())start();
				else stop();
			}
					break;
			case 'W':
			case 'w':
				if (driver_inside && engine.is_started())
				{
					accellerate();
				}
				break;
			case Enter:
				if (driver_inside)get_out();
				else get_in(); break;
			case Escape:
				if (control.free_wheeling_thread.joinable())control.free_wheeling_thread.join();
				stop();
				get_out();
				cout << "Hava La'vista baby" << endl; break;
			}
			//if (driver_inside)panel();
			std::this_thread::sleep_for(0.1s);
			if (speed > 0 && !control.free_wheeling_thread.joinable())control.free_wheeling_thread = std::thread(&Car::free_wheeling, this);
			else if (control.free_wheeling_thread.joinable())control.free_wheeling_thread.join();
		} while (key != Escape);
	}

	void accellerate()
	{
		speed += acceleration;
		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for(1s);
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
		cout << "Engine:\n"; engine.info();
		cout << "Tank:\n"; tank.info();
		cout << "Engine is " << (engine.is_started() ? "started" : "stopped") << endl;
		cout << "Speed:\t" << speed << "km/h\n";
		cout << "MaxSpeed:\t" << MAX_SPEED << "km/h\n";
	}
};

//#define TANK_CHECK
//#define ENGINE_CHECK

void main()
{
	setlocale(LC_ALL, "");
#ifdef TANK_CHECK
	Tank tank(35);
	tank.info();
	cout << "\n-----------------------------\n";
	tank.set_fuel_level(30);
	tank.info();
	cout << "\n-----------------------------\n";
	tank.set_fuel_level(40);
	tank.info();
#endif // TANK_CHECK

#ifdef ENGINE_CHECK
	Engine engine(10);
	engine.info();
#endif // ENGINE_CHECK

	//Car car(Engine(10), Tank(80));
	Car car(10, 50, 250);
	//car.info();

	cout << "Press Enter to get in" << endl;
	cout << "Press F to to fill the car" << endl;


}