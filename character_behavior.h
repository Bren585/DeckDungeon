#pragma once
#include "BLIB\string.h"

class character_model;
namespace cb {}

class character_behavior {
	friend character_model;
private:
	enum {
		state_enter,
		state_main,
		state_exit,
		state_finished
	} state;

	virtual void enter		(					) {}
	virtual void main		(float elapsed_time	) {}
	virtual bool condition	(					) = 0;
	virtual void exit		(					) {}

protected:
	character_model* parent;

public:
	character_behavior(character_model* parent) : parent(parent), state(state_enter) {}
	virtual ~character_behavior() = default;

	void update(float elapsed_time);
	bool finished() { return state == state_finished; }

	virtual void add_required() {}
};

class character_animation_behavior : public character_behavior {
	const string animation_name;
	const bool loop;
	virtual bool condition();
protected:
	void enter() override;
protected:
	character_animation_behavior(character_model* parent, string animation, bool loop = false) : 
		character_behavior(parent), 
		animation_name(animation),
		loop(loop)
	{}
public:
	virtual ~character_animation_behavior() = default;
};

class character_idle_behavior : public character_animation_behavior {
	const int backlog_max;
	bool condition() override;
protected:
	character_idle_behavior(character_model* parent, string animation, int backlog = 1) :
		character_animation_behavior(parent, animation, true),
		backlog_max(backlog)
	{}
public:
	virtual ~character_idle_behavior() = default;
};

class character_behavior_set : public character_behavior {
	bool condition() override { return true; } 
	virtual void add_required() override = 0;
protected:
	character_behavior_set(character_model* parent) : character_behavior(parent) {}
public:
	virtual ~character_behavior_set() = default;
};
