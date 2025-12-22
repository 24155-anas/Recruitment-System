#pragma once
#include <iostream>
#include <vector>
using namespace std;

template<typename T>
struct Node {
    Node* right;
    Node* left;
    T val;
    int height;
    Node(T _val) : right(nullptr), left(nullptr), val(_val), height(1) {}
};

template<typename T>
class AVLTree {
private:
    Node<T>* root;

    Node<T>* insert(Node<T>* root, T val) {
        if (root == nullptr) {
            root = new Node<T>(val);
            return root;
        }

        if (val < root->val) {
            root->left = insert(root->left, val);
        }
        else if (val > root->val) {
            root->right = insert(root->right, val);
        }
        return root;
    }

    int tree_height(Node<T>* root) {
        if (root == nullptr) {
            return -1;
        }
        int leftH = tree_height(root->left);
        int rightH = tree_height(root->right);
        return max(leftH, rightH) + 1;
    }

    int getNodeHeight(Node<T>* node) {
        if (node == nullptr) {
            return 0;
        }
        return node->height;
    }

    int getBF(Node<T>* node) {
        if (node == nullptr) {
            return 0;
        }
        int bf = getNodeHeight(node->left) - getNodeHeight(node->right);
        return bf;
    }

    Node<T>* rightRotation(Node<T>* b) {
        Node<T>* a = b->left;
        Node<T>* c = a->right;

        a->right = b;
        b->left = c;

        b->height = max(getNodeHeight(b->right), getNodeHeight(b->left)) + 1;
        a->height = max(getNodeHeight(a->right), getNodeHeight(a->left)) + 1;

        return a;
    }

    Node<T>* leftRotation(Node<T>* a) {
        Node<T>* b = a->right;
        Node<T>* c = b->left;

        b->left = a;
        a->right = c;

        a->height = max(getNodeHeight(a->right), getNodeHeight(a->left)) + 1;
        b->height = max(getNodeHeight(b->right), getNodeHeight(b->left)) + 1;

        return b;
    }

    Node<T>* balancedInsert(Node<T>* root, T val) {
        if (root == nullptr) {
            root = new Node<T>(val);
            return root;
        }

        if (val < root->val) {
            root->left = balancedInsert(root->left, val);
        }
        else if (val > root->val) {
            root->right = balancedInsert(root->right, val);
        }

        root->height = 1 + max(getNodeHeight(root->right), getNodeHeight(root->left));
        int bf = getBF(root);

        // Left-Left case
        if (bf > 1 && val < root->left->val) {
            root = rightRotation(root);
        }

        // Right-Right case
        if (bf < -1 && val > root->right->val) {
            root = leftRotation(root);
        }

        // Left-Right case
        if (bf > 1 && val > root->left->val) {
            root->left = leftRotation(root->left);
            root = rightRotation(root);
        }

        // Right-Left case
        if (bf < -1 && val < root->right->val) {
            root->right = rightRotation(root->right);
            root = leftRotation(root);
        }

        return root;
    }

    Node<T>* search(Node<T>* root, T val) {
        if (root == nullptr) {
            return nullptr;
        }

        if (val > root->val) {
            return search(root->right, val);
        }
        else if (val < root->val) {
            return search(root->left, val);
        }
        else {
            return root;
        }
    }

    Node<T>* deleteNode(Node<T>* root, T val) {
        if (root == nullptr) {
            return nullptr;
        }
        if (val > root->val) {
            root->right = deleteNode(root->right, val);
        }
        else if (val < root->val) {
            root->left = deleteNode(root->left, val);
        }
        else {
            // Case 1: leaf node
            if (root->right == nullptr && root->left == nullptr) {
                delete root;
                return nullptr;
            }
            // Case 2: 1 child
            else if (root->right && root->left == nullptr) {
                Node<T>* temp = root->right;
                delete root;
                return temp;
            }
            else if (root->left && root->right == nullptr) {
                Node<T>* temp = root->left;
                delete root;
                return temp;
            }
            // Case 3: 2 children
            else {
                Node<T>* parentOfSucc = root;
                Node<T>* succ = root->right;
                while (succ->left) {
                    parentOfSucc = succ;
                    succ = succ->left;
                }
                root->val = succ->val;

                if (parentOfSucc->right == succ) {
                    parentOfSucc->right = succ->right;
                }
                else {
                    parentOfSucc->left = succ->right;
                }
                delete succ;
            }
        }
        return root;
    }

    void inOrder(Node<T>* root, vector<T>& result) {
        if (root == nullptr) {
            return;
        }
        inOrder(root->left, result);
        result.push_back(root->val);
        inOrder(root->right, result);
    }

    void clearTree(Node<T>* root) {
        if (root == nullptr) {
            return;
        }
        clearTree(root->left);
        clearTree(root->right);
        delete root;
    }


    
public:
    AVLTree() : root(nullptr) {}

    ~AVLTree() {
        clearTree(root);
    }

    void insert(T val) {
        root = balancedInsert(root, val);
    }

    void insertUnbalanced(T val) {
        root = insert(root, val);
    }

    bool contains(T val) {
        return search(root, val) != nullptr;
    }

    void remove(T val) {
        root = deleteNode(root, val);
    }

    int height() {
        return tree_height(root);
    }

    vector<T> inOrderTraversal() {
        vector<T> result;
        inOrder(root, result);
        return result;
    }

    void printInOrder() {
        vector<T> elements = inOrderTraversal();
        for (const T& elem : elements) {
            cout << elem << " ";
        }
        cout << endl;
    }

    bool isEmpty() {
        return root == nullptr;
    }

    T* find(T val) {
        Node<T>* result = search(root, val);
        return result ? &(result->val) : nullptr;
    }

    vector<T> getAllElements() {
        return inOrderTraversal();
    }
};