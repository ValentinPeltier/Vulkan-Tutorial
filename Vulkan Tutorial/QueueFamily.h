#ifndef QUEUEFAMILY_H
#define QUEUEFAMILY_H

struct QueueFamilyIndices {
	int graphicsFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0;
	}
};

#endif