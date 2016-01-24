///
/// Copyright 2015, Hannes Janetzek
///
/// Port of the implementation by:
/// Bram Stein - http://www.bramstein.com/projects/typeset/
///
/// Preserve Knuth and Plass line breaking algorithm in C++
///
/// Licensed under the new BSD License.
///

#include "lineLayout.h"

#include <memory>
#include <vector>

#define LINE_INFINITY 10000

namespace alfons {

struct WordWrap {
    struct Breakpoint {
        uint32_t index;
        double demerits;
        double ratio;

        int line;
        int fitnessClass;
        int totalWidth;
        int totalStretch;
        int totalShrink;
        Breakpoint* prev;
        int refs = 1;

        Breakpoint() {}

        Breakpoint(uint32_t index, double demerits, double ratio, int line,
                   int fitnessClass, int totalWidth, int totalStretch,
                   int totalShrink, Breakpoint* previous)
            : index(index),
              demerits(demerits),
              ratio(ratio),
              line(line),
              fitnessClass(fitnessClass),
              totalWidth(totalWidth),
              totalStretch(totalStretch),
              totalShrink(totalShrink),
              prev(previous) {}

        void print() {
            printf("BREAK index:%d line:%d fit:%d width:%d, stretch:%d shrink:%d, "
                   "dem:%f\n",
                   index, line, fitnessClass, totalWidth, totalStretch, totalShrink,
                   demerits);
        }
    };

    struct Row {
        int breakpoint;
        double ratio;
        float width;
        Row(int breakpoint, double ratio, float width)
            : breakpoint(breakpoint), ratio(ratio), width(width) {}
    };

    // Class representing a glyph or character.  Boxes have a fixed width that
    // doesn't change.
    struct BoxData {
        uint32_t id;
    };

    struct GlueData {
        int stretch = 0;
        int shrink = 0;
    };

    // Class representing a bit of glue.  Glue has a preferred width, but it can
    // stretch up to an additional distance, and can shrink by a certain amount.
    // Line breaks can be placed at any point where glue immediately follows a
    // box.
    struct PenaltyData {
        int penalty = 0;
        bool flagged = 0;
    };

    struct Box {

        bool isBox = false;
        bool isGlue = false;
        bool isPenalty = false;
        int width;

        union {
            BoxData b;
            GlueData g;
            PenaltyData p;
        };

        Box(int width, uint32_t id) : isBox(true), width(width) { b.id = id; }

        Box(int width, int penalty, bool flagged) : isPenalty(true), width(width) {
            p.penalty = penalty;
            p.flagged = flagged;
        }

        Box(int width, int stretch, int shrink) : isGlue(true), width(width) {
            g.stretch = stretch;
            g.shrink = shrink;
        }

        void print() const {
            if (isBox) {
                printf("BOX width:%d, CHAR:%d\n", width, b.id);
            } else if (isGlue) {
                printf("GLUE width:%d, stretch:%d shrink:%d\n", width, g.stretch,
                       g.shrink);
            } else if (isPenalty) {
                printf("PENALTY width:%d, penalty:%d\n", width, p.penalty);
            }
        }
    };

    std::vector<Box> nodes;
    std::vector<Breakpoint*> activeNodes;
    std::vector<int> lineLengths;

    int sumWidth = 0;
    int sumStretch = 0;
    int sumShrink = 0;

    // OPTIONS for mainloop
    int tolerance = 1;
    double flaggedDemerit = 100;
    double fitnessDemerit = 100;
    double lineDemerit = 10;

    WordWrap(int lineLength = 40) { lineLengths.push_back(lineLength); }

    inline void addClosingPenalty() {
        // addPenalty(0, LINE_INFINITY, 0);
        addGlue(0, LINE_INFINITY, 0);
        addPenalty(0, -LINE_INFINITY, 1);
    }

    inline void addPenalty(int width, int penalty, bool flagged = false) {
        nodes.emplace_back(width, penalty, flagged);
    }

    inline void addGlue(int width, int stretch, int shrink) {
        nodes.emplace_back(width, stretch, shrink);
    }

    inline void addBox(int width, uint32_t id) { nodes.emplace_back(width, id); }

    double computeCost(const Breakpoint& start, const Box& node);

    void mainloop(const uint32_t index, const Box& node);

    std::vector<Row> breakLines(void);

    bool wrapLine(LineLayout& layout, float width, float maxWidth,
                  Alignment align, glm::vec2& resultSize);

    ~WordWrap() {
        while (pool) {
            Breakpoint* p = pool->prev;
            delete pool;
            pool = p;
        }
    }

private:
    Breakpoint* newBreakpoint(uint32_t index, double demerits, double ratio,
                              int line, int fitnessClass, int totalWidth,
                              int totalStretch, int totalShrink,
                              Breakpoint* previous);
    void drop(Breakpoint*);

    Breakpoint* pool = nullptr;
};
}
