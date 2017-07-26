﻿/***********************************************************************************
**
** Weather.h
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#ifndef WEATHER_H
#define WEATHER_H
//----------------------------------------------------------------------------------
#include "WeatherEffect.h"
//----------------------------------------------------------------------------------
//Класс для работы с погодой
class CWeather
{
	//Тип вогоды
	SETGET(uchar, Type, 0);
	//Максимальное количество эффектов на экране
	SETGET(uchar, Count, 0);
	//Текущее количество эффектов на экране
	SETGET(uchar, CurrentCount, 0);
	//Температура
	SETGET(uchar, Temperature, 0);
	//Время отрисовки
	SETGET(uint, Timer, 0);
	//Направление ветра
	SETGET(char, Wind, 0);
	//Таймер смены ветра
	SETGET(uint, WindTimer, 0);
	//Время последней смены анимации погоды
	SETGET(uint, LastTick, 0);
	//Скорость общей симуляции зависит от этого значения. Больше - медленней
	SETGET(float, SimulationRatio, 37.0f);

public:
	CWeather();
	~CWeather() { m_Effects.clear(); }

	//Погодные эффекты
	deque<CWeatherEffect> m_Effects;
	
	//Выключить погодные эффекты
	void Reset();

	//Сгенерировать эффекты
	void Generate();

	//Отрисовать погодные эффекты
	void Draw(const int &x, const int &y);
};
//----------------------------------------------------------------------------------
extern CWeather g_Weather;
//----------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------
