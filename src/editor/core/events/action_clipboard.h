#pragma once
#include "action_event.h"
#include <vector>

namespace Karbon {

class ActionClipboard {
public:
    ActionClipboard(int capacity = 100) : m_capacity(capacity) {}
    void PushAction(std::unique_ptr<ActionEvent> action) {
        if (m_actions.size() >= m_capacity) {
            m_actions.erase(m_actions.begin());
        }
        m_actions.push_back(std::move(action));
    }
    void UndoAction() {
        if (m_actions.empty()) return;
        m_actions.back()->undo();
        idx--;
    }
    void RedoAction() {
        if (idx >= m_actions.size()) return;
        m_actions[idx]->undo(); // Assuming undo() can also redo, or you have a separate redo() method
        idx++;
    }
private:
    std::vector<std::unique_ptr<ActionEvent>> m_actions;
    int idx;
    int m_capacity;
};
}