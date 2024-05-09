/*
 * 2024 DKU OS Lab2
 * Student ID : 32221741
 * Name : Park Seonghyeon
*/

#include "bst_impl.h"

void BST::insert(int key, int value){
	Node* newNode = new Node;
	
	if (root == nullptr) {
		newNode->key = key;
		newNode->value = value;
		newNode->upd_cnt = 1;
		newNode->left = nullptr;
		newNode->right = nullptr;
		root = newNode;
		return;
	}
	
	Node* current = root;
	while (1) {
		if (key == current->key) {
			current->upd_cnt += 1;
			return;
		}
		while (key < current->key) {
			if (current->left == nullptr) {
				newNode->key = key;
				newNode->value = value;
				newNode->upd_cnt = 1;
				newNode->left = nullptr;
				newNode->right = nullptr;
				current->left = newNode;
				return;
			}
			else {
				current = current->left;
			}
		}
		while (key > current->key) {
			if (current->right == nullptr) {
				newNode->key = key;
				newNode->value = value;
				newNode->upd_cnt = 1;
				newNode->left = nullptr;
				newNode->right = nullptr;
				current->right = newNode;
				return;
			}
			else {
				current = current->right;
			}
		}
	}
}

int BST::lookup(int key) {
	
	return 0;
} 

void BST::remove(int key){
	
}

void BST::traversal(KVC* traverse_arr) {
	traverse_arr_ = traverse_arr;
	arr_index_ = 0;
	Node* current = root;
	Node* parent = nullptr;
	
	if (root == nullptr) return;
	
	while (current != nullptr) {
		if (current->left == nullptr) {
			traverse_arr_[arr_index_] = KVC{current->key, current->value, current->upd_cnt};
			arr_index_ += 1;
		}
		else {
			parent = current->left;
			while (parent->right != nullptr && parent->right != current) parent = parent->right;
			if (parent->right == nullptr) {
				parent->right = current;
				current = current->left;
			}
			else {
				parent->right = nullptr;
				traverse_arr_[arr_index_] = KVC{current->key, current->value, current->upd_cnt};
				arr_index_ += 1;
				current = current->right;
			}
		}
	}
}

void CoarseBST::insert(int key, int value){
	// Todo
}

int CoarseBST::lookup(int key) {
	// Todo
	return 0;
} 

void CoarseBST::remove(int key){
	// Todo
}

void CoarseBST::traversal(KVC* traverse_arr) {
    // Todo
}

void FineBST::insert(int key, int value){
	// Todo
}

int FineBST::lookup(int key) {
	// Todo
	return 0;
} 

void FineBST::remove(int key){
	// Todo
}

void FineBST::traversal(KVC* traverse_arr) {
    // Todo
}
