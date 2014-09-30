#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <list>

#define BIG_NUMBER 1000
#define WORLD_WIDTH 5
#define WORLD_HEIGHT 5

struct Node {
	int x;
	int y;
	float cost;
	float heuristicCost;
	Node *parent;
	int difficulty;
};

struct World {
	Node **nodes;
	int width;
	int height;
};

struct Color {
	int r;
	int g;
	int b;
};

// Log a tag and a message
void log(std::string _tag, std::string _message) {
	std::cout << _tag << ": " << _message << std::endl;
}

Node* createNode(int _x, int _y, float _cost) {
	// alloc a node
	Node *node = (Node*) malloc(sizeof(Node));
	// define its position
	node->x = _x;
	node->y = _y;
	// define its cost
	node->cost = _cost;
	// define its heuristic cost
	node->heuristicCost = 0.0;
	node->parent = NULL;
	node->difficulty = 0;

	return node;
}

// Print a node
void printNode(Node *_node) {
	std::stringstream message;
	message << "(" << _node->x << ", " << _node->y << ")";
	message << " c: " << _node->cost << " | hc: " << _node->heuristicCost;
	log("printNode", message.str());
}

// Return true if node1's heuristic cost is lower than node2's heuristic cost
bool compareTwoNodes(Node *_node1, Node *_node2) {
	return _node1->heuristicCost < _node2->heuristicCost;
}

// get the cheaper node of a list
Node* getCheaperNode(std::list<Node*> _list) {
	_list.sort(compareTwoNodes);

	return _list.front();
}

// Get node's neighbours
std::list<Node*> getNodeNeighbours(World *_world, Node *_node, int _step) {
	std::list<Node*> neighbours;

	std::stringstream message;

	int offsetX = -1;
	int offsetY = -1;
	int x = -1;
	int y = -1;
	int id = -1;
	for (int i = 0; i < _step * 2 + 1; ++i) {
		for (int j = 0; j < _step * 2 + 1; ++j) {
			offsetX = j - _step;
			offsetY = i - _step;
			// skip the node itself
			if ((offsetX == 0 && offsetY == 0) == false) {
				x = _node->x + offsetX;
				y = _node->y + offsetY;
				// avoid out of range coordinates
				if ((x >= 0 && x < _world->width) && (y >= 0 && y < _world->height)) {
					id = y * _world->width + x;

					Node *node = _world->nodes[id];

					neighbours.push_back(node);
				}
			}
		}
	}

	return neighbours;
}

// Return true if a node is in the list
bool isNodeInList(Node *_node, std::list<Node*> _list) {
	bool found = false;

	std::list<Node*>::iterator n = _list.begin();
	while (n != _list.end() && found == false) {
		if (*n == _node) {
			found = true;
		}

		++n;
	}

	return found;
}

// Get the distance between two nodes
float getDistanceBetweenTwoNodes(Node *_node1, Node *_node2) {
	float distance = 0.0;

	// get the vector
	int i = _node2->x - _node1->x;
	int j = _node2->y - _node1->y;

	// compute the distance
	distance = sqrt(i * i + j * j);

	return distance;
}

// Create a world defined by its width and height
World* createWorld(int _width, int _height) {
	// alloc the world
	World *world = (World*) malloc(sizeof(World));

	// define its dimension
	world->width = _width;
	world->height = _height;

	// alloc its nodes
	int nbNodes = world->width * world->height;
	world->nodes = (Node**) malloc(nbNodes * sizeof(Node));

	int id = -1;
	// create its nodes
	for (int i = 0; i < world->height; ++i) {
		for (int j = 0; j < world->width; ++j) {
			id = i * world->width + j;
			
			world->nodes[id] = createNode(j, i, 0.0);
		}
	}

	return world;
}

// Destroy a world
void destroyWorld(World *_world) {
	// free each node
	int nbNodes = _world->width * _world->height;
	for (int i = 0; i < nbNodes; ++i) {
		free(_world->nodes[i]);
	}

	// free nodes
	free(_world->nodes);

	// free world
	free(_world);
}

// Print a world
void printWorld(World *_world) {
	int id = -1;
	for (int i = 0; i < _world->height; ++i) {
		for (int j = 0; j < _world->width; ++j) {
			id = i * _world->width + j;

			Node *node = _world->nodes[id];

			std::string mark = "0";
			if (node->difficulty == BIG_NUMBER) {
				mark = "X";
			} else if (node->difficulty > 0) {
				mark = "H";
			}

			std::cout << mark;
		}

		std::cout << std::endl;
	}
}

// Print a world with a path
void printWorldWithPath(World *_world, std::list<Node*> _path) {
	int id = -1;
	for (int i = 0; i < _world->height; ++i) {
		for (int j = 0; j < _world->width; ++j) {
			id = i * _world->width + j;

			Node *node = _world->nodes[id];

			std::string mark = "0";
			if (node->difficulty == BIG_NUMBER) {
				mark = "X";
			} else if (node->difficulty > 0) {
				mark = "H";
			}

			if (isNodeInList(node, _path)) {
				mark = "-";
			}

			std::cout << mark;
		}

		std::cout << std::endl;
	}
}

// Draw a world with a path in a ppm image
World* readWorldFromImage(std::string _filename) {
	World *world = NULL;

	std::ifstream file(_filename.c_str(), std::ios::in);

	if (file) {
		std::string line;

		// get PX
		std::getline(file, line);

		// get comment
		std::getline(file, line);

		// get dimensions
		std::getline(file, line);
		int pos = line.find(" ");
		std::string strWidth = line.substr(0, pos);
		int width = atoi(strWidth.c_str());
		std::string strHeight = line.substr(pos + 1);
		int height = atoi(strHeight.c_str());

		// get color capacity
		std::getline(file, line);

		// create the world
		world = createWorld(width, height);

		int nbNodes = width * height;

		int parts[nbNodes * 3];
		
		int j = 0;
		// fulfill the world
		while (getline(file, line)) {

			parts[j] = atoi(line.c_str());

			++j;
		}

		Color **colors = (Color**) malloc(nbNodes * sizeof(Color));
		for (int i = 0; i < nbNodes; ++i) {
			colors[i] = (Color*) malloc(sizeof(Color));
			colors[i]->r = parts[i * 3 + 0];
			colors[i]->g = parts[i * 3 + 1];
			colors[i]->b = parts[i * 3 + 2];

			std::cout << colors[i]->r << ", " << colors[i]->g << ", " << colors[i]->b << std::endl;
		
			if (colors[i]->r == 0) {
				world->nodes[i]->difficulty = BIG_NUMBER;
			}
		}

		for (int i = 0; i < nbNodes; ++i) {
			free(colors[i]);
		}

		free(colors);

		file.close();
	}

	return world;
}

// Draw a world with a path in a ppm image
void drawWorldWithPathInImage(
	World *_world, std::list<Node*> _path, std::string _filename) {
	std::ofstream file(_filename.c_str(), std::ios::out | std::ios::trunc);

	if (file) {
		file << "P3" << std::endl;
		file << "# CREATOR: Made with a-star" << std::endl;
		file << _world->width << " " << _world->height << std::endl;
		file << "255" << std::endl;

		for (int i = 0; i < _world->height; ++i) {
			for (int j = 0; j < _world->width; ++j) {

				int id = i * _world->width + j;

				Node *node = _world->nodes[id];

				std::string mark = "255\n255\n255";
				if (node->difficulty == BIG_NUMBER) {
					mark = "0\n0\n0";
				} else if (node->difficulty > 0) {
					mark = "0\n0\n255";
				}

				if (isNodeInList(node, _path)) {
					mark = "0\n255\n0";
				}

				file << mark;
				/*if ((i == (_world->height - 1) && j == (_world->height - 1)) == false) {
					file << "\n";
				}*/
					file << "\n";
			}
		}

		file.close();
	}
}

// Apply A* to find a path
std::list<Node*> aStar(World *_world, Node *_start, Node *_finish) {
	std::list<Node*> path;

	std::stringstream message;

	printNode(_start);
	printNode(_finish);

	float distance = getDistanceBetweenTwoNodes(_start, _finish);
	_start->heuristicCost = _start->cost + distance;

	message.str("");
	message << _start->heuristicCost;
	log("aStar", message.str());

	std::list<Node*> openList;
	openList.push_back(_start);

	std::list<Node*> closeList;

	message.str("");
	message << _start->x;
	log("aStar", message.str());

	while (openList.empty() == false) {
		// get cheaper node
		Node *node = getCheaperNode(openList);
		// remove it from the open list
		openList.remove(node);

		// add it to close list
		closeList.push_back(node);

		// if the node is the goal
		if (node == _finish) {
			log("aStar", "finish found");
		} else {
			int step = 1;
			// get node's neighbours
			std::list<Node*> neighbours = getNodeNeighbours(_world, node, step);
			// for each neighbours
			for (int i = (neighbours.size() - 1); i >= 0; i--) {
				Node *neighbour = neighbours.back();
				neighbours.pop_back();

				// if the neighbour hasn't been processed
				if (isNodeInList(neighbour, closeList) == false) {
					// compute its new cost
					//float newCost = node->cost + neighbour->cost;
					float distranceNodeNeighbour = getDistanceBetweenTwoNodes(node, neighbour);
					float newCost = node->cost + neighbour->difficulty + distranceNodeNeighbour;

					bool neightbourInOpenList = isNodeInList(neighbour, openList);
					// if the neighbour is interesting
					if (neightbourInOpenList == false || newCost < neighbour->cost) {
						// save the parent
						neighbour->parent = node;
						// the neightbout takes its new cost
						neighbour->cost = newCost;
						float distanceToFinish = getDistanceBetweenTwoNodes(neighbour, _finish);
						// the neighbour takes its new heuristic cost
						neighbour->heuristicCost = neighbour->cost + distanceToFinish;

						// put it in the open list
						if (neightbourInOpenList == false) {
							openList.push_back(neighbour);
						}
					}
				}
			}
		}
	}

	log("aStar", "------------");
	
	Node *n = _finish;
	while (n->parent != NULL) {
		path.push_back(n);

		n = n->parent;
	}
	path.push_back(n);

	return path;
}

//////////
// MAIN //
//////////

int main() {
	log("main", "Begin");

	//World *world = createWorld(WORLD_WIDTH, WORLD_HEIGHT);
	World *world = readWorldFromImage("world.ppm");

	/*for (int i = 0; i < WORLD_WIDTH - 1; ++i) {
		world->nodes[1 * WORLD_WIDTH + i]->difficulty = BIG_NUMBER;
	}

	world->nodes[WORLD_WIDTH + 3]->difficulty = 10;

	for (int i = 1; i < WORLD_WIDTH; ++i) {
		world->nodes[(WORLD_HEIGHT - 2) * WORLD_WIDTH + i]->difficulty = BIG_NUMBER;
	}*/

	printWorld(world);

	Node *start = world->nodes[0];
	Node *finish = world->nodes[world->height * world->width - 1];
	std::list<Node*> path = aStar(world, start, finish);

	/*for (std::list<Node*>::iterator n = path.begin(); n != path.end(); ++n) {
		printNode(*n);
	}*/

	printWorldWithPath(world, path);

	drawWorldWithPathInImage(world, path, "path.ppm");

	destroyWorld(world);

	log("main", "End");

	return 0;
}
