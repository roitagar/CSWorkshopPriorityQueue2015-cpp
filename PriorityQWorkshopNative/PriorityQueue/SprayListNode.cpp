#include "SprayListPriorityQueue.h"

SprayListNode::SprayListNode(int value, int height)
{
	this->value = value;
	this->_fullyLinked = false;
	this->_marked = false;
	this->_height = height;
	this->next = new SprayListNode*[height+1];

}

SprayListNode::~SprayListNode()
{
	delete[] this->next;
}

int SprayListNode::TopLevel()
{
	return _height;
}

bool SprayListNode::IsFullyLinked()
{
	return _fullyLinked;
}

bool SprayListNode::IsMarked()
{
	return _marked;
}

void SprayListNode::SetFullyLinked()
{
	_fullyLinked = true;
}

void SprayListNode::SetMark()
{
	_marked = true;
}
