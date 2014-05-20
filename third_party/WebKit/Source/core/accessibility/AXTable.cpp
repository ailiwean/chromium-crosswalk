/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "core/accessibility/AXTable.h"

#include "core/accessibility/AXObjectCache.h"
#include "core/accessibility/AXTableCell.h"
#include "core/accessibility/AXTableColumn.h"
#include "core/accessibility/AXTableRow.h"
#include "core/dom/ElementTraversal.h"
#include "core/html/HTMLCollection.h"
#include "core/html/HTMLTableCaptionElement.h"
#include "core/html/HTMLTableCellElement.h"
#include "core/html/HTMLTableColElement.h"
#include "core/html/HTMLTableElement.h"
#include "core/html/HTMLTableRowElement.h"
#include "core/html/HTMLTableSectionElement.h"
#include "core/rendering/RenderTableCell.h"

namespace WebCore {

using namespace HTMLNames;

AXTable::AXTable(RenderObject* renderer)
    : AXRenderObject(renderer)
    , m_headerContainer(nullptr)
    , m_isAXTable(true)
{
}

AXTable::~AXTable()
{
}

void AXTable::init()
{
    AXRenderObject::init();
    m_isAXTable = isTableExposableThroughAccessibility();
}

PassRefPtr<AXTable> AXTable::create(RenderObject* renderer)
{
    return adoptRef(new AXTable(renderer));
}

bool AXTable::hasARIARole() const
{
    if (!m_renderer)
        return false;

    AccessibilityRole ariaRole = ariaRoleAttribute();
    if (ariaRole != UnknownRole)
        return true;

    return false;
}

bool AXTable::isAXTable() const
{
    if (!m_renderer)
        return false;

    return m_isAXTable;
}

static bool elementHasAriaRole(const Element* element)
{
    if (!element)
        return false;

    const AtomicString& ariaRole = element->fastGetAttribute(roleAttr);
    return (!ariaRole.isNull() && !ariaRole.isEmpty());
}

bool AXTable::isDataTable() const
{
    if (!m_renderer || !node())
        return false;

    // Do not consider it a data table is it has an ARIA role.
    if (hasARIARole())
        return false;

    // When a section of the document is contentEditable, all tables should be
    // treated as data tables, otherwise users may not be able to work with rich
    // text editors that allow creating and editing tables.
    if (node() && node()->rendererIsEditable())
        return true;

    // This employs a heuristic to determine if this table should appear.
    // Only "data" tables should be exposed as tables.
    // Unfortunately, there is no good way to determine the difference
    // between a "layout" table and a "data" table.

    RenderTable* table = toRenderTable(m_renderer);
    Node* tableNode = table->node();
    if (!isHTMLTableElement(tableNode))
        return false;

    // Do not consider it a data table if any of its descendants have an ARIA role.
    HTMLTableElement* tableElement = toHTMLTableElement(tableNode);
    if (elementHasAriaRole(tableElement->tHead()))
        return false;
    if (elementHasAriaRole(tableElement->tFoot()))
        return false;

    RefPtrWillBeRawPtr<HTMLCollection> bodies = tableElement->tBodies();
    for (unsigned bodyIndex = 0; bodyIndex < bodies->length(); ++bodyIndex) {
        Element* bodyElement = bodies->item(bodyIndex);
        if (elementHasAriaRole(bodyElement))
            return false;
    }

    RefPtrWillBeRawPtr<HTMLCollection> rows = tableElement->rows();
    for (unsigned rowIndex = 0; rowIndex < rows->length(); ++rowIndex) {
        Element* rowElement = rows->item(rowIndex);
        if (elementHasAriaRole(rowElement))
            return false;
        if (rowElement->hasTagName(trTag)) {
            HTMLTableRowElement* row = static_cast<HTMLTableRowElement*>(rowElement);
            RefPtrWillBeRawPtr<HTMLCollection> cells = row->cells();
            for (unsigned cellIndex = 0; cellIndex < cells->length(); ++cellIndex) {
                if (elementHasAriaRole(cells->item(cellIndex)))
                    return false;
            }
        }
    }

    // If there is a caption element, summary, THEAD, or TFOOT section, it's most certainly a data table
    if (!tableElement->summary().isEmpty() || tableElement->tHead() || tableElement->tFoot() || tableElement->caption())
        return true;

    // if someone used "rules" attribute than the table should appear
    if (!tableElement->rules().isEmpty())
        return true;

    // if there's a colgroup or col element, it's probably a data table.
    if (Traversal<HTMLTableColElement>::firstChild(*tableElement))
        return true;

    // go through the cell's and check for tell-tale signs of "data" table status
    // cells have borders, or use attributes like headers, abbr, scope or axis
    table->recalcSectionsIfNeeded();
    RenderTableSection* firstBody = table->firstBody();
    if (!firstBody)
        return false;

    int numCols = firstBody->numColumns();
    int numRows = firstBody->numRows();

    // If there's only one cell, it's not a good AXTable candidate.
    if (numRows == 1 && numCols == 1)
        return false;

    // If there are at least 20 rows, we'll call it a data table.
    if (numRows >= 20)
        return true;

    // Store the background color of the table to check against cell's background colors.
    RenderStyle* tableStyle = table->style();
    if (!tableStyle)
        return false;
    Color tableBGColor = tableStyle->visitedDependentColor(CSSPropertyBackgroundColor);

    // check enough of the cells to find if the table matches our criteria
    // Criteria:
    //   1) must have at least one valid cell (and)
    //   2) at least half of cells have borders (or)
    //   3) at least half of cells have different bg colors than the table, and there is cell spacing
    unsigned validCellCount = 0;
    unsigned borderedCellCount = 0;
    unsigned backgroundDifferenceCellCount = 0;
    unsigned cellsWithTopBorder = 0;
    unsigned cellsWithBottomBorder = 0;
    unsigned cellsWithLeftBorder = 0;
    unsigned cellsWithRightBorder = 0;

    Color alternatingRowColors[5];
    int alternatingRowColorCount = 0;

    int headersInFirstColumnCount = 0;
    for (int row = 0; row < numRows; ++row) {

        int headersInFirstRowCount = 0;
        for (int col = 0; col < numCols; ++col) {
            RenderTableCell* cell = firstBody->primaryCellAt(row, col);
            if (!cell)
                continue;
            Node* cellNode = cell->node();
            if (!cellNode)
                continue;

            if (cell->width() < 1 || cell->height() < 1)
                continue;

            validCellCount++;

            bool isTHCell = cellNode->hasTagName(thTag);
            // If the first row is comprised of all <th> tags, assume it is a data table.
            if (!row && isTHCell)
                headersInFirstRowCount++;

            // If the first column is comprised of all <th> tags, assume it is a data table.
            if (!col && isTHCell)
                headersInFirstColumnCount++;

            // in this case, the developer explicitly assigned a "data" table attribute
            if (isHTMLTableCellElement(*cellNode)) {
                HTMLTableCellElement& cellElement = toHTMLTableCellElement(*cellNode);
                if (!cellElement.headers().isEmpty() || !cellElement.abbr().isEmpty()
                    || !cellElement.axis().isEmpty() || !cellElement.scope().isEmpty())
                    return true;
            }

            RenderStyle* renderStyle = cell->style();
            if (!renderStyle)
                continue;

            // If the empty-cells style is set, we'll call it a data table.
            if (renderStyle->emptyCells() == HIDE)
                return true;

            // If a cell has matching bordered sides, call it a (fully) bordered cell.
            if ((cell->borderTop() > 0 && cell->borderBottom() > 0)
                || (cell->borderLeft() > 0 && cell->borderRight() > 0))
                borderedCellCount++;

            // Also keep track of each individual border, so we can catch tables where most
            // cells have a bottom border, for example.
            if (cell->borderTop() > 0)
                cellsWithTopBorder++;
            if (cell->borderBottom() > 0)
                cellsWithBottomBorder++;
            if (cell->borderLeft() > 0)
                cellsWithLeftBorder++;
            if (cell->borderRight() > 0)
                cellsWithRightBorder++;

            // If the cell has a different color from the table and there is cell spacing,
            // then it is probably a data table cell (spacing and colors take the place of borders).
            Color cellColor = renderStyle->visitedDependentColor(CSSPropertyBackgroundColor);
            if (table->hBorderSpacing() > 0 && table->vBorderSpacing() > 0
                && tableBGColor != cellColor && cellColor.alpha() != 1)
                backgroundDifferenceCellCount++;

            // If we've found 10 "good" cells, we don't need to keep searching.
            if (borderedCellCount >= 10 || backgroundDifferenceCellCount >= 10)
                return true;

            // For the first 5 rows, cache the background color so we can check if this table has zebra-striped rows.
            if (row < 5 && row == alternatingRowColorCount) {
                RenderObject* renderRow = cell->parent();
                if (!renderRow || !renderRow->isBoxModelObject() || !toRenderBoxModelObject(renderRow)->isTableRow())
                    continue;
                RenderStyle* rowRenderStyle = renderRow->style();
                if (!rowRenderStyle)
                    continue;
                Color rowColor = rowRenderStyle->visitedDependentColor(CSSPropertyBackgroundColor);
                alternatingRowColors[alternatingRowColorCount] = rowColor;
                alternatingRowColorCount++;
            }
        }

        if (!row && headersInFirstRowCount == numCols && numCols > 1)
            return true;
    }

    if (headersInFirstColumnCount == numRows && numRows > 1)
        return true;

    // if there is less than two valid cells, it's not a data table
    if (validCellCount <= 1)
        return false;

    // half of the cells had borders, it's a data table
    unsigned neededCellCount = validCellCount / 2;
    if (borderedCellCount >= neededCellCount
        || cellsWithTopBorder >= neededCellCount
        || cellsWithBottomBorder >= neededCellCount
        || cellsWithLeftBorder >= neededCellCount
        || cellsWithRightBorder >= neededCellCount)
        return true;

    // half had different background colors, it's a data table
    if (backgroundDifferenceCellCount >= neededCellCount)
        return true;

    // Check if there is an alternating row background color indicating a zebra striped style pattern.
    if (alternatingRowColorCount > 2) {
        Color firstColor = alternatingRowColors[0];
        for (int k = 1; k < alternatingRowColorCount; k++) {
            // If an odd row was the same color as the first row, its not alternating.
            if (k % 2 == 1 && alternatingRowColors[k] == firstColor)
                return false;
            // If an even row is not the same as the first row, its not alternating.
            if (!(k % 2) && alternatingRowColors[k] != firstColor)
                return false;
        }
        return true;
    }

    return false;
}

bool AXTable::isTableExposableThroughAccessibility() const
{
    // The following is a heuristic used to determine if a
    // <table> should be exposed as an AXTable. The goal
    // is to only show "data" tables.

    if (!m_renderer)
        return false;

    // If the developer assigned an aria role to this, then we
    // shouldn't expose it as a table, unless, of course, the aria
    // role is a table.
    if (hasARIARole())
        return false;

    return isDataTable();
}

void AXTable::clearChildren()
{
    AXRenderObject::clearChildren();
    m_rows.clear();
    m_columns.clear();

    if (m_headerContainer) {
        m_headerContainer->detachFromParent();
        m_headerContainer = nullptr;
    }
}

void AXTable::addChildren()
{
    if (!isAXTable()) {
        AXRenderObject::addChildren();
        return;
    }

    ASSERT(!m_haveChildren);

    m_haveChildren = true;
    if (!m_renderer || !m_renderer->isTable())
        return;

    RenderTable* table = toRenderTable(m_renderer);
    AXObjectCache* axCache = m_renderer->document().axObjectCache();

    // Go through all the available sections to pull out the rows and add them as children.
    table->recalcSectionsIfNeeded();
    RenderTableSection* tableSection = table->topSection();
    if (!tableSection)
        return;

    RenderTableSection* initialTableSection = tableSection;
    while (tableSection) {

        HashSet<AXObject*> appendedRows;
        unsigned numRows = tableSection->numRows();
        for (unsigned rowIndex = 0; rowIndex < numRows; ++rowIndex) {

            RenderTableRow* renderRow = tableSection->rowRendererAt(rowIndex);
            if (!renderRow)
                continue;

            AXObject* rowObject = axCache->getOrCreate(renderRow);
            if (!rowObject->isTableRow())
                continue;

            AXTableRow* row = toAXTableRow(rowObject);
            // We need to check every cell for a new row, because cell spans
            // can cause us to miss rows if we just check the first column.
            if (appendedRows.contains(row))
                continue;

            row->setRowIndex(static_cast<int>(m_rows.size()));
            m_rows.append(row);
            if (!row->accessibilityIsIgnored())
                m_children.append(row);
            appendedRows.add(row);
        }

        tableSection = table->sectionBelow(tableSection, SkipEmptySections);
    }

    // make the columns based on the number of columns in the first body
    unsigned length = initialTableSection->numColumns();
    for (unsigned i = 0; i < length; ++i) {
        AXTableColumn* column = toAXTableColumn(axCache->getOrCreate(ColumnRole));
        column->setColumnIndex((int)i);
        column->setParent(this);
        m_columns.append(column);
        if (!column->accessibilityIsIgnored())
            m_children.append(column);
    }

    AXObject* headerContainerObject = headerContainer();
    if (headerContainerObject && !headerContainerObject->accessibilityIsIgnored())
        m_children.append(headerContainerObject);
}

AXObject* AXTable::headerContainer()
{
    if (m_headerContainer)
        return m_headerContainer.get();

    AXMockObject* tableHeader = toAXMockObject(axObjectCache()->getOrCreate(TableHeaderContainerRole));
    tableHeader->setParent(this);

    m_headerContainer = tableHeader;
    return m_headerContainer.get();
}

AXObject::AccessibilityChildrenVector& AXTable::columns()
{
    updateChildrenIfNecessary();

    return m_columns;
}

AXObject::AccessibilityChildrenVector& AXTable::rows()
{
    updateChildrenIfNecessary();

    return m_rows;
}

void AXTable::columnHeaders(AccessibilityChildrenVector& headers)
{
    if (!m_renderer)
        return;

    updateChildrenIfNecessary();

    unsigned colCount = m_columns.size();
    for (unsigned k = 0; k < colCount; ++k) {
        AXObject* header = toAXTableColumn(m_columns[k].get())->headerObject();
        if (!header)
            continue;
        headers.append(header);
    }
}

void AXTable::cells(AXObject::AccessibilityChildrenVector& cells)
{
    if (!m_renderer)
        return;

    updateChildrenIfNecessary();

    int numRows = m_rows.size();
    for (int row = 0; row < numRows; ++row) {
        AccessibilityChildrenVector rowChildren = m_rows[row]->children();
        cells.appendVector(rowChildren);
    }
}

unsigned AXTable::columnCount()
{
    updateChildrenIfNecessary();

    return m_columns.size();
}

unsigned AXTable::rowCount()
{
    updateChildrenIfNecessary();

    return m_rows.size();
}

AXTableCell* AXTable::cellForColumnAndRow(unsigned column, unsigned row)
{
    updateChildrenIfNecessary();
    if (column >= columnCount() || row >= rowCount())
        return 0;

    // Iterate backwards through the rows in case the desired cell has a rowspan and exists in a previous row.
    for (unsigned rowIndexCounter = row + 1; rowIndexCounter > 0; --rowIndexCounter) {
        unsigned rowIndex = rowIndexCounter - 1;
        AccessibilityChildrenVector children = m_rows[rowIndex]->children();
        // Since some cells may have colspans, we have to check the actual range of each
        // cell to determine which is the right one.
        for (unsigned colIndexCounter = std::min(static_cast<unsigned>(children.size()), column + 1); colIndexCounter > 0; --colIndexCounter) {
            unsigned colIndex = colIndexCounter - 1;
            AXObject* child = children[colIndex].get();
            ASSERT(child->isTableCell());
            if (!child->isTableCell())
                continue;

            pair<unsigned, unsigned> columnRange;
            pair<unsigned, unsigned> rowRange;
            AXTableCell* tableCellChild = toAXTableCell(child);
            tableCellChild->columnIndexRange(columnRange);
            tableCellChild->rowIndexRange(rowRange);

            if ((column >= columnRange.first && column < (columnRange.first + columnRange.second))
                && (row >= rowRange.first && row < (rowRange.first + rowRange.second)))
                return tableCellChild;
        }
    }

    return 0;
}

AccessibilityRole AXTable::roleValue() const
{
    if (!isAXTable())
        return AXRenderObject::roleValue();

    return TableRole;
}

bool AXTable::computeAccessibilityIsIgnored() const
{
    AXObjectInclusion decision = defaultObjectInclusion();
    if (decision == IncludeObject)
        return false;
    if (decision == IgnoreObject)
        return true;

    if (!isAXTable())
        return AXRenderObject::computeAccessibilityIsIgnored();

    return false;
}

String AXTable::title() const
{
    if (!isAXTable())
        return AXRenderObject::title();

    String title;
    if (!m_renderer)
        return title;

    // see if there is a caption
    Node* tableElement = m_renderer->node();
    if (isHTMLTableElement(tableElement)) {
        HTMLTableCaptionElement* caption = toHTMLTableElement(tableElement)->caption();
        if (caption)
            title = caption->innerText();
    }

    // try the standard
    if (title.isEmpty())
        title = AXRenderObject::title();

    return title;
}

} // namespace WebCore
