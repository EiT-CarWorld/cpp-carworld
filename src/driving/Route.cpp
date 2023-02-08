#include "Route.h"

bool Route::verifyRoute() {
    for(int i = 0; i+1 < paths.size(); i++) {
        Node* i1 = paths[i]->m_b;
        Node* i2 = paths[i]->m_oneway ? i1 : paths[i]->m_a;
        Node* I1 = paths[i+1]->m_a;
        Node* I2 = paths[i+1]->m_oneway ? I1 : paths[i+1]->m_b;

        if(i1 == I1 || i1 == I2 || i2 == I1 || i2 == I2)
            continue; // The paths are connected in a legal way

        return false;
    }
    return true;
}