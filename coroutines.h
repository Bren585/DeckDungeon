#pragma once
#define ALLOW_COROUTINES int coroutine_step = -1
#define BEGIN_COROUTINE switch (coroutine_step) {case -1:
#define YIELD_WHILE_IMPL(label, cond)	\
	do {								\
		coroutine_step = label;			\
		[[fallthrough]];				\
		case label:;					\
		if (cond) return;				\
	} while (0)
#define YIELD_WHILE(cond) YIELD_WHILE_IMPL(__COUNTER__, cond)
#define END_COROUTINE }

#define WAIT(duration)					\
	do {								\
		timer = 0;						\
		YIELD_WHILE(timer < duration);	\
	} while (0)

#define SET_CHECKPOINT(value) [[fallthrough]]; case value + 10000:
#define GO_TO_CHECKPOINT(value) coroutine_step = (int)value + 10000; return;

#define COIT coroutine_iterator
#define ALLOW_COROUTINE_ITERATOR int COIT = 0;

#define YIELD_SUBTASK(task, ...)								\
	do {														\
		BLIB::manager::add(new task(__VA_ARGS__));				\
		YIELD_WHILE(BLIB::manager::find_first_of_type<task>());	\
	} while (0)