/*
 * unp_log.h
 *
 * Логирование
 *
 *  Created on: 20.08.2015
 *      Author: kudr
 */

#ifndef UNP_LOG_H_
#define UNP_LOG_H_

#include <stdarg.h>
#include <stdio.h>

#include <string>

namespace UNP {

//! Типы сообщений
enum LogMessageType {
	LMT_DEBUG     = -1,  //!< Отладочные сообщения (в версии release игнорируются)
	
	//! \name Информационные сообщения (отличаются цветом в консоли в классе LoggerSys)
	//! {
	LMT_NOTE      = 0,   //!< Простые информационные сообщения
	LMT_HIGHLIGHT = 1,   //!< Подсвеченные сообщения (выделяются синим цветом)
	LMT_SUCCESS   = 2,   //!< Сообщения об успешном действии (выделяются зелёным цветом)
	//! }
	
	LMT_WARNING   = 100, //!< Предупреждения
	LMT_ERROR     = 101  //!< Ошибки
};

//! Базовый класс логирования 
class Logger {
public:
	Logger();
	virtual ~Logger();
	
	//! Логирование произвольного сообщения
	virtual void AddV(LogMessageType type, const char *format, va_list ap) = 0;
	
	//! Логирование произвольного сообщения
	void Add(LogMessageType type, const char *format, ...) __attribute__((format (__printf__, 3, 4)));
	
	//! Логирование системной ошибки с кодом err
	void AddSysError(int err, const char *prefix);
	
	//! Логирование системной ошибки с кодом err в формате:
	//! prefix 'name': сообщение
	void AddSysError(int err, const char *prefix, const char *name);
	
	void AddError(const char *format, ...)     __attribute__((format (__printf__, 2, 3)));
	void AddWarning(const char *format, ...)   __attribute__((format (__printf__, 2, 3)));
	void AddNote(const char *format, ...)      __attribute__((format (__printf__, 2, 3)));
	void AddHighlight(const char *format, ...) __attribute__((format (__printf__, 2, 3)));
	void AddSuccess(const char *format, ...)   __attribute__((format (__printf__, 2, 3)));
	void AddDebug(const char *format, ...)     __attribute__((format (__printf__, 2, 3)));
};

//! Настройки LoggerSysSets
class LoggerSysSets {
public:
	std::string appName; //!< Имя приложения
	bool quiet;          //!< Логировать только ошибки (LMT_ERROR)
	
	bool toStd;          //!< Логировать на stdout/stderr
	bool toJournal;      //!< Логировать в системный журнал
	bool enColorConsole; //!< Включить раскраску в консоли
	bool stdAddTime;     //!< Добавлять время при выводе на std. Время монотонное, считается с момента запуска.
	
	class ToFile {
	public:
		std::string fileName;     //!< Имя файла (пустая строка - не логировать)
		unsigned recordsCountMax; //!< Максимальное количество записей (0 - без ограничений)
		                          //!< При превышении будет создана резервная копия, файл будет очищен
		unsigned backupCountMax;  //!< Максимальное количество резервных копий
		bool truncBefore;         //!< Очищать файл перед началом логирования
		bool noTime;              //!< Не добавлять время (и дату) перед каждой записью
		bool sync;                //!< Вызывать fsync() после каждой записи
		
	public:
		ToFile() : recordsCountMax(1000), backupCountMax(4), truncBefore(false), noTime(false), sync(false) {}
	} toFile;            //!< Настройки логирования в файл
	
public:
	//!< Конструктор (по умолчанию логирование только на std)
	LoggerSysSets() : quiet(false), toStd(true), toJournal(false), enColorConsole(true), stdAddTime(false) {
	}
	
	virtual ~LoggerSysSets() {}
};

//! Потокобезопасное логирование на stdout/stderr, в системный журнал, в файл
class LoggerSys : public Logger {
public:
	/*!
	 * @param sets Настройки
	 * @return
	 */
	LoggerSys(const LoggerSysSets &sets);
	
	/*!
	 * @param appName        Имя приложения
	 * @param toStd          На std
	 * @param toJournal      В системный журнал
	 * @param quiet          Логировать только ошибки (LMT_ERROR)
	 * @param enColorConsole Включить раскраску в консоли
	 */
	LoggerSys(const char *appName, bool toStd, bool toJournal, bool quiet = false, bool enColorConsole = true);
	
	virtual ~LoggerSys();
	
	LoggerSys(const LoggerSys&) = delete;
	void operator=(const LoggerSys&) = delete;
	
	void AddV(LogMessageType type, const char *format, va_list ap) override;
	
protected: // read-only
	LoggerSysSets sets;
	
private:
	FILE *logF;
	unsigned recordsCount;
	int lockFd;
	int64_t tStart_ns;
	
private:
	void Init();
	void BackupLog();
};

//! Потокобезопасное логирование в строку
class LoggerString : public Logger {
public:
	LoggerString();
	virtual ~LoggerString();
	
	void AddV(LogMessageType type, const char *format, va_list ap) override;
	
	std::string GetClear();
	
	LoggerString(const LoggerString&) = delete;
	void operator=(const LoggerString&) = delete;
	
private:
	void *impl;
};

/*!
 * Вывести в лог дамп в текстовом виде
 * 
 * @param prefix  Префикс в заголовке (м.б. NULL)
 * @param msg     Сообщение в заголовке (м.б. NULL)
 * @param buf     Буфер
 * @param len     Длина
 */
void LogDump(const char *prefix, const char *msg, const void *buf, size_t len);

/*!
 * Вывести в лог дамп в текстовом виде с подсветкой различий
 * 
 * @param prefix   Префикс в заголовке (м.б. NULL)
 * @param msg      Сообщение в заголовке (м.б. NULL)
 * @param bufCur   Буфер с текущими данными
 * @param lenCur   Длина буфера с текущими данными
 * @param bufPrev  Буфер с предыдущими данными (если NULL, то эквивалентно LogDump())
 * @param lenPrev  Длина буфера с предыдущими данными
 */
void LogDumpDiff(const char *prefix, const char *msg, const void *bufCur, size_t lenCur, const void *bufPrev, size_t lenPrev);

}
#endif
