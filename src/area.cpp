#include "area.hpp"

AreaAllocator::AreaAllocator() 
{
    m_width  = 0;
    m_height = 0;
}

void AreaAllocator::Initialize(int width, int height) 
{
    m_width = width;
    m_height = height;

    m_free_areas.emplace_back(new Area{0,0,width,height});
}

int AreaAllocator::GetTotalWidth() { return m_width; }
int AreaAllocator::GetTotalHeight() { return m_height;}
int AreaAllocator::GetTotalAreaSize() { return m_width * m_height; }

int AreaAllocator::GetFreeAreaCount() { return static_cast<int>(m_free_areas.size()); }
int AreaAllocator::GetFreeAreaSize() { return accumulateAreaSize(m_free_areas); }

int AreaAllocator::GetAllocatedAreaCount() { return static_cast<int>(m_allocated_areas.size()); }
int AreaAllocator::GetAllocatedAreaSize() { return accumulateAreaSize(m_allocated_areas); }

Area* AreaAllocator::Allocate(int width, int height) 
{
    Area *oldArea = getFreeArea(width, height);

    if(!oldArea) {
        collapseFreeAreas();
        oldArea = getFreeArea(width, height);
    }

    if(!oldArea) {
        return nullptr;
    }

    auto newArea = new Area {oldArea->x, oldArea->y, width, height};
    m_allocated_areas.emplace_front(newArea);


    if(oldArea->width > width) {
        // Add an area to the right of newly allocated area
        m_free_areas.emplace_back(new Area {oldArea->x + width, oldArea->y, oldArea->width - width, height});
    }
    if(oldArea->height > height) {
        // Add an area below the newly allocated area
        m_free_areas.emplace_back(new Area {oldArea->x, oldArea->y + height, width, oldArea->height - height});
    }
    if(oldArea->width > width && oldArea->height > height) {
        // Add an area diagonally to the right and below the newly allocated area
        m_free_areas.emplace_back(new Area {oldArea->x + width, oldArea->y + height, oldArea->width - width, oldArea->height - height});
    }
    return newArea;
}

void AreaAllocator::Free(Area *area) 
{
    m_allocated_areas.remove(area);
    m_free_areas.emplace_back(area);
}

int AreaAllocator::accumulateAreaSize(const AreaList &areaList) const
{
    int size = 0;
    for(const auto& area : areaList) {
        size += area->width*area->height;
    }
    return size;
}

// check if the area is there
Area* AreaAllocator::getFreeArea(int width, int height) 
{
    Area* oldArea = nullptr;
    auto foundIt = m_free_areas.FindArea(width, height);
    if(foundIt != m_free_areas.end()) {
        oldArea = *foundIt;
        m_free_areas.erase(foundIt);
    }
    return oldArea;
}

void AreaAllocator::collapseFreeAreas() 
{
    if( m_free_areas.size() < 2 )
    {
        return;
    }

    int collapsed = 0;
    do { // a wild do-while appears!
        collapsed = 0;
        AreaList collapsed_areas;
        while(!m_free_areas.empty()) {
            auto first = m_free_areas.front();
            m_free_areas.pop_front();

            while(!m_free_areas.empty()) {
                auto other = m_free_areas.FindAdjacent(*first);
                if(other != m_free_areas.end()) {
                    first->CombineWith(**other);
                    delete *other;
                    m_free_areas.erase(other);
                    ++collapsed;
                } else {
                    break;
                }
            }
            collapsed_areas.emplace_back(first);
        }
        m_free_areas = collapsed_areas;
    } while(collapsed > 0);
}
