#pragma once

namespace LR {
	enum ActionType {
		SHIFT,
		REDUCE
	};

	struct Action {
		ActionType type;
		int val;

		Action(ActionType type = SHIFT, int val = -1) : type(type), val(val) {
		}
	};
}