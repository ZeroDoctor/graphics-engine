#include "build_order.hpp"

struct Area {
    int x;
    int y;
    int width;
    int height;

    bool IsAdjacent(const Area &other) const {
        if (x == other.x && width == other.width && y + height == other.y) {
            // Other is immediately below
            return true;
        }

        if (x == other.x && width == other.width && other.y + other.height == y) {
            // Other is immediately above
            return true;
        }

        if(y == other.y && height == other.height && x + width == other.x) {
            // Other is immediately to the right
            return true;
        }

        if(y == other.y && height == other.height && other.x + other.width == x) {
            // Other is immediately to the left
            return true;
        }

    }

    bool CombineWith(const Area& other)
    {
        if (x == other.x && width == other.width && y + height == other.y) {
            // Other is immediately below
            height += other.height;
            return true;
        }

        if (x == other.x && width == other.width && other.y + other.height == y) {
            // Other is immediately above
            y -= other.height;
            height += other.height;
            return true;
        }

        if(y == other.y && height == other.height && x + width == other.x) {
            // Other is immediately to the right
            width += other.width;
            return true;
        }

        if(y == other.y && height == other.height && other.x + other.width == x) {
            // Other is immediately to the left
            x -= other.width;
            width += other.width;
            return true;
        }

        return false;
    }
};

typedef std::list<Area*> AreaPtrList_t;

class AreaList : public AreaPtrList_t
{
public:
    AreaList::const_iterator FindAdjacent(const Area &area)
    {
        for(auto it = cbegin(); it != cend(); ++it) {
            if(area.IsAdjacent(**it)) {
                return it;
            }
        }
        return cend();
    }

    AreaList::const_iterator FindArea(int width, int height)
    {
        auto foundIt = cend();

        for(auto it = cbegin(); it != cend(); ++it) {
            if((*it)->width >= width && (*it)->height >= height) {
                foundIt = it;
                break;
            }
        }

        return foundIt;
    }
};

class AreaAllocator
{
public:
    AreaAllocator();

    void Initialize(int width, int height);

    int GetTotalWidth();
    int GetTotalHeight();
    int GetTotalAreaSize();
    int GetFreeAreaCount();
    int GetFreeAreaSize();
    int GetAllocatedAreaCount();
    int GetAllocatedAreaSize();

    Area* Allocate(int width, int height);

    void Free(Area *area);

protected:
    int m_width;
    int m_height;

    AreaList m_free_areas;
    AreaList m_allocated_areas;

    int accumulateAreaSize(const AreaList &areaList) const;
    Area *getFreeArea(int width, int height);
    void collapseFreeAreas();
};
