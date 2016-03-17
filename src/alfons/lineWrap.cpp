//
// Port of the implementation by:
// Bram Stein - http://www.bramstein.com/projects/typeset/
// Copyright 2009-2010, Bram Stein
//
// Preserve Knuth and Plass line breaking algorithm in C++
//
// Licensed under the new BSD License.
//
//
// Copyright 2015, Hannes Janetzek
//

#include "lineWrap.h"

#include <glm/vec2.hpp>

#include <algorithm>
#include <stdint.h>
#include <cmath>
#include <limits>
#include <string>

#include "logger.h"

using std::pow;
using std::abs;

//#define DEBUG_WRAP

#ifndef DEBUG_WRAP
#ifdef LOGD
#undef LOGD
#define LOGD(...)
#endif
#endif

#define Infinity std::numeric_limits<double>::infinity()

namespace alfons {

typedef WordWrap::Breakpoint Breakpoint;
typedef WordWrap::Row Row;

// Compute adjustment ratio for the line between start and end
double WordWrap::computeCost(const Breakpoint& start, const Box& node) {
    int width = sumWidth - start.totalWidth;
    LOGD("\tcompute cost:length:%d  %d/%d", width, sumWidth, start.totalWidth);

    // Get the length of the current line; if the line_lengths list is too short,
    // the last value is always used for subsequent lines.
    int lineLength = (start.line < (int)(lineLengths.size() - 1))
                         ? lineLengths[start.line]
                         : lineLengths[lineLengths.size() - 1];

    if (node.isPenalty) {
        width += node.width;
        LOGD("\tm add penaltiy %d", node.width);
    }

    // Compute how much the contents of the line would have to be stretched or
    // shrunk to fit into the available space.
    if (width < lineLength) {
        // Calculate the stretch ratio
        int stretch = sumStretch - start.totalStretch;

        LOGD("\tLine too short: %d, stretch = %d  %d/%d", lineLength - width,
             stretch, sumStretch, start.totalStretch);

        if (stretch > 0)
            return (lineLength - width) / double(stretch);
        else
            return LINE_INFINITY;

    } else if (width > lineLength) {
        // Calculate the shrink ratio
        int shrink = sumShrink - start.totalShrink;

        LOGD("\tLine too long: %d, shrink = %d  %d/%d", lineLength - width,
             shrink, sumShrink, start.totalShrink);

        if (shrink > 0)
            return (lineLength - width) / double(shrink);
        else
            return LINE_INFINITY;
    }

    // Exactly the right width!
    return 0;
}

struct Candidate {
    // BreakpointRef active;
    Breakpoint* active;
    double demerits;
    double ratio;
};

Breakpoint* WordWrap::newBreakpoint(uint32_t index, double demerits,
                                    double ratio, int line, int fitnessClass,
                                    int totalWidth, int totalStretch,
                                    int totalShrink, Breakpoint* previous) {
    if (!pool)
        return new Breakpoint(index, demerits, ratio, line, fitnessClass,
                              totalWidth, totalStretch, totalShrink, previous);
    Breakpoint* p = pool;
    pool = pool->prev;
    p->refs = 1;
    p->index = index;
    p->demerits = demerits;
    p->ratio = ratio;
    p->line = line;
    p->fitnessClass = fitnessClass;
    p->totalWidth = totalWidth;
    p->totalStretch = totalStretch;
    p->totalShrink = totalShrink;
    p->prev = previous;
    return p;
}

void WordWrap::drop(Breakpoint* t) {
    while (t && --t->refs <= 0) {
        Breakpoint* p = t->prev;

        // release to pool
        t->prev = pool;
        pool = t;

        t = p;
    }
}

void WordWrap::mainloop(const uint32_t index, const Box& node) {
    auto iter = activeNodes.begin();
    if (iter == activeNodes.end()) {
        LOGI("EMPTY");
        return;
    }

    auto active = *iter;

    Candidate candidate[4];

    // The inner loop iterates through all the active nodes with (line <
    // currentLine) and then breaks out to insert the new active node candidates
    // before looking at the next active nodes for the next lines. The result of
    // this is that the active node list is always sorted by line number.
    LOGD("run %d - active nodes:%d", index, (int)activeNodes.size());

    while (active) {
        LOGD(">>> outer");

        candidate[0].demerits = Infinity;
        candidate[1].demerits = Infinity;
        candidate[2].demerits = Infinity;
        candidate[3].demerits = Infinity;
        candidate[0].active = nullptr;
        candidate[1].active = nullptr;
        candidate[2].active = nullptr;
        candidate[3].active = nullptr;

        // Iterate through the linked list of active nodes to find new potential
        // active nodes and deactivate current active nodes.
        while (active) {
            // auto nextActive = it++;
            // active = *iter;
            LOGD("\t>>> inner <<<");

            if (!active)
                break;

            int currentLine = active->line + 1;

            float ratio = computeCost(*active, node);
            LOGD("\tline:%d  ratio:%f", active->line, ratio);

            // Deactive nodes when the distance between the current active node and
            // the current node becomes too large (i.e. it exceeds the stretch limit
            // and the stretch ratio becomes negative) or when the current node is a
            // forced break (i.e. the end of the paragraph when we want to remove all
            // active nodes, but possibly have a final candidate active node -- if the
            // paragraph can be set using the given tolerance value.)
            if (ratio < -1 || (node.isPenalty && node.p.penalty == -LINE_INFINITY)) {
                // NB: reached an index at which the active breakpoint cannot be
                // the start of the line anymore
                iter = activeNodes.erase(iter);
                // Drop at end of loop, if no new references were added
                active->refs--;
            } else {
                iter++;
            }

            if ((ratio >= -1 && ratio <= tolerance)) {
                // Compute demerits and fitness class
                double demerits = pow(lineDemerit + 100.0 * pow(abs(ratio), 3.0), 2.0);
                // double demerits;

                if (node.isPenalty && node.p.penalty > 0) {
                    // Positive penalty
                    demerits += pow(node.p.penalty, 2.0);
                } else if (node.isPenalty && node.p.penalty != -LINE_INFINITY) {
                    // Negative penalty but not a forced break
                    demerits -= pow(node.p.penalty, 2.0);
                }

                if (node.isPenalty && nodes[active->index].isPenalty)
                    demerits += (flaggedDemerit * node.p.flagged *
                                 nodes[active->index].p.flagged);

                // Figure out the fitness class of this line (tight, loose, very tight
                // or very loose).
                int fitnessClass = 0;
                if (ratio < -0.5)
                    fitnessClass = 0;
                else if (ratio <= 0.5)
                    fitnessClass = 1;
                else if (ratio <= 1)
                    fitnessClass = 2;
                else
                    fitnessClass = 3;

                // Add a fitness penalty to the demerits if the fitness classes of two
                // adjacent lines differ too much.  If two consecutive lines are in very
                // different fitness classes, add to the demerit score for this break.
                if (abs(fitnessClass - active->fitnessClass) > 1) {
                    demerits += fitnessDemerit;
                }

                // Add the total demerits of the active node to get the total demerits
                // of this candidate node.
                demerits += active->demerits;

                LOGD("\tdemerits %d, %f  -  %f", fitnessClass, demerits,
                     candidate[fitnessClass].demerits);

                // Record a feasible break from A to B
                // Only store the best candidate for each fitness class
                if (demerits < candidate[fitnessClass].demerits) {
                    auto& b = candidate[fitnessClass];

                    if (b.active)
                        drop(b.active);

                    active->refs++;
                    b.active = active;
                    b.ratio = ratio;
                    b.demerits = demerits;
                }
            }

            if (active->refs == 0)
                drop(active);

            active = nullptr;

            if (iter != activeNodes.end()) {
                // Continue whith next Breakpoint
                active = *iter;

                // Stop iterating through active nodes to insert new candidate active
                // nodes in the active list before moving on to the active nodes for the
                // next line.
                // TODO: The Knuth and Plass paper suggests a conditional for
                // currentLine < j0. This means paragraphs with identical line lengths
                // will not be sorted by line number. Find out if that is a desirable
                // outcome.  For now I left this out, as it only adds minimal overhead
                // to the algorithm and keeping the active node list sorted has a higher
                // priority.
                if (active->line >= currentLine) {
                    LOGD("exit inner: %d >= %d", active->line, currentLine);
                    break;
                }
            }
        }
        LOGD("<<< outer");

        // Add width, stretch and shrink values from the current break point up to
        // the next box or forced penalty.
        int width = sumWidth;
        int stretch = sumStretch;
        int shrink = sumShrink;
        for (uint32_t i = index; i < nodes.size(); i++) {
            auto& n = nodes[i];
            if (n.isGlue) {
                width += n.width;
                stretch += n.g.stretch;
                shrink += n.g.shrink;
            } else if (n.isBox || (n.p.penalty == -LINE_INFINITY && i > index)) {
                break;
            }
        }

        for (int fitnessClass = 0; fitnessClass < 4; fitnessClass++) {
            auto& c = candidate[fitnessClass];

            if (c.demerits == Infinity)
                continue;

            auto newNode =
                newBreakpoint(index, c.demerits, c.ratio, c.active->line + 1,
                              fitnessClass, width, stretch, shrink, c.active);

#ifdef DEBUG_WRAP
            LOGD("Insert breakpoint:");
            newNode->print();
#endif
            if (active) {
                // LOGD("insert before");
                // TEST: should insert before
                iter = activeNodes.insert(iter, newNode);
                iter++;

            } else {
                // LOGD("push back");
                activeNodes.push_back(newNode);
            }
        }
    }
}

std::vector<Row> WordWrap::breakLines() {
    activeNodes.clear();
    sumWidth = 0;
    sumStretch = 0;
    sumShrink = 0;

    // Add an active node for the start of the paragraph.
    activeNodes.push_back(newBreakpoint(0, // index
                                        0, // demerits
                                        0, // ratio
                                        0, // line
                                        0, // class
                                        0, // width
                                        0, // stretch,
                                        0, // shrink,
                                        nullptr));
    uint32_t index = 0;

    for (const auto& node : nodes) {
#ifdef DEBUG_WRAP
        LOGD("");
        node.print();
#endif
        if (node.isBox) {
            // LOGD("add box %d", index);
            sumWidth += node.width;
        } else if (node.isGlue) {
            // LOGD("add glue %d", index);

            if (index > 0 && nodes[index - 1].isBox)
                mainloop(index, node);

            sumWidth += node.width;
            sumStretch += node.g.stretch;
            sumShrink += node.g.shrink;
        } else if (node.isPenalty && node.p.penalty != LINE_INFINITY) {
            // LOGD("add penalty %d", index);
            mainloop(index, node);
        }
#ifdef DEBUG_WRAP
        LOGD(">>>");
        for (auto& node : activeNodes)
            node->print();
        LOGD("<<<");
#endif

        index++;

        if (activeNodes.empty())
            break;
    }

    LOGD("Result nodes: %d", (int)activeNodes.size());

    // Find the best active node (the one with the least total demerits.)
    Breakpoint* best = nullptr;
    Breakpoint* t = nullptr;
    std::vector<Row> breaks;

#if 1
    for (auto& brk : activeNodes) {
        // node->print();

        LOGD(">> %d  - %d", (int)brk->demerits, (int)(brk->demerits * brk->line));
        t = brk->prev; //.get();
        while (t) {
            LOGD(">> %d  - %d", (int)t->demerits, (int)(t->demerits * t->line));
            t = t->prev; //.get();
        }

        if (brk->index < index - 1) {
            LOGI("missing chain %d", brk->index);
            continue;
        }

        if (!best) {
            best = brk; //.get();
            continue;
        }
        if (brk->demerits < best->demerits) {
            best = brk; //.get();
            continue;
        }

        // if (brk->demerits * brk->line < best->demerits * brk->line) {
        //   best = brk.get();
        //   continue;
        // }

        // if (brk->line < best->line) {
        //   best = brk;
        //   continue;
        // }
        // if (brk->line == best->line && brk->demerits < best->demerits){
        //   best = brk;
        //   continue;
        // }
    };

    LOGD("Result ptr: %p", best);

#ifdef DEBUG_WRAP
    if (best) {
        LOGD("Picked:");
        best->print();
    }
#endif

    while (best && best->prev) {
        breaks.emplace_back(best->index, best->ratio,
                            best->totalWidth - best->prev->totalWidth);

        best = best->prev;
    }

    // Free remaining nodes
    for (auto& brk : activeNodes)
        drop(brk);

    LOGD("Result breaks: %d", (int)breaks.size());
#else
    for (auto& brk : activeNodes) {
        // node->print();

        breaks.push_back(brk->index);

        LOGD(">> %d  - %d", (int)brk->demerits, (int)(brk->demerits * brk->line));
        t = brk->prev;
        while (t) {
            LOGD(">> %d  - %d", t->demerits, (int)(t->demerits * t->line));
            t = t->prev;
        }

        t = brk->prev;
        while (t) {
            breaks.push_back(t->index);
            t = t->prev;
        }
    }
#endif
    std::reverse(breaks.begin(), breaks.end());
    return breaks;
}

bool WordWrap::wrapLine(LineLayout& layout, float width, float maxWidth,
                        Alignment align, glm::vec2& resultSize,
                        std::vector<glm::vec2> _offsets) {
    nodes.clear();

    int wordStart = 0;
    float wordWidth = 0;
    bool block = align == Alignment::block;
    bool centered = align == Alignment::middle;

    float space = 0;
    float stretch = 0;
    float shrink = 0;

    int numWords = 0;

    for (int i = 0, n = layout.shapes().size(); i < n; i++) {
        auto& c = layout.shapes()[i];

        if (c.canBreak) {
            if (space == 0 && c.isSpace) {
                space = layout.advance(c);
                stretch = space * 2;
                shrink = space / 2;
            }
            float s = space;
            int pos = i;

            if (!c.isSpace) {
                wordWidth += layout.advance(c);
                s = 0;
                pos += 1;
            }

            wordWidth += 0.5;

            if (wordWidth > width) {
                width = wordWidth;
                if (width > maxWidth) {
                    log("MAX %f", width);
                    return false;
                }
            }

            addBox(wordWidth, pos);
            numWords++;

            if (block) {
                addGlue(s, stretch, shrink);
            } else {
                addGlue(0, width, 0);
                addPenalty(0, 0, false);
                addGlue(s, -width, 0);
            }
            wordStart = i;
            wordWidth = 0;
        } else {
            wordWidth += layout.advance(c);
        }
    }
    if (wordWidth > 0) {
        addBox(wordWidth, wordStart);
        numWords++;

        if (block) {
            addGlue(space, stretch, shrink);
        } else {
            addGlue(0, width, 0);
            addPenalty(0, 0, false);
            addGlue(space, -width, 0);
        }
    }

    addClosingPenalty();

    lineLengths[0] = width;

    tolerance = 10;
    flaggedDemerit = 100;
    lineDemerit = 10;
    fitnessDemerit = block || centered ? 1000 : 0;

    auto rows = breakLines();

    if (rows.empty()) {
        log("EMPTY %d", width);

        return false;
    }
    int nodeStart = 0;
    glm::vec2 offset(0);

    wordStart = 0;

    _offsets.clear();
    offset.y -= (rows.size() - 1) * layout.height();

    maxWidth = 0;
    for (auto& row : rows) {
        if (row.width > maxWidth)
            maxWidth = row.width;
    }
    int glyphCount = 0;
    for (auto& g : layout.shapes())
        if (!g.isSpace)
            glyphCount++;

    _offsets.reserve(glyphCount);

    for (auto& row : rows) {
        float lineWordSpacing = space;
        if (block)
            lineWordSpacing += row.ratio * (row.ratio < 0 ? shrink : stretch);

        int offsetStart = _offsets.size();

        // if (centered)
        //   offset.x = (maxWidth - row.width) / 2;

        for (int i = nodeStart; i < row.breakpoint; i++) {
            auto& box = nodes[i];
            if (box.isGlue) {
                if (box.width > 0)
                    offset.x += lineWordSpacing;
            } else if (box.isBox && box.width > 0) {
                for (uint32_t j = wordStart; j < box.b.id; j++) {
                    auto& c = layout.shapes()[j];
                    if (!c.isSpace) {
                        _offsets.emplace_back(offset + c.position);
                        offset.x += layout.advance(c);
                    }
                }
                wordStart = box.b.id;
            }
        }

        if (centered) {
            //log("row width %f %f", row.width, offset.x);
            int offsetEnd = _offsets.size();
            float justify = (maxWidth - offset.x) / 2;
            for (int j = offsetStart; j < offsetEnd; j++)
                _offsets[j].x += justify;
        }

        nodeStart = row.breakpoint + 1;

        offset.x = 0;
        offset.y += layout.height();
    }

    resultSize.x = maxWidth;
    resultSize.y = rows.size() * layout.height();

    return true;
}
}
