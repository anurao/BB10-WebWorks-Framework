/*
 *  Copyright 2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

var MAX_NUM_ITEMS_IN_PORTRAIT_PEEK_MODE = 7,
    MAX_NUM_ITEMS_IN_LANDSCAPE_PEEK_MODE = 3,
    MAX_NUM_ITEMS_IN_PEEK_MODE = 7,
    PEEK_MODE_TRANSLATE_X = -121,
    FULL_MENU_TRANSLATE_X = -569,
    HIDDEN_MENU_TRANSLATE_X = 0,
    state = {
        HIDE: 0,
        PEEK: 1,
        VISIBLE: 2,
        DRAGEND: 3
    },
    menu,
    menuCurrentState = state.HIDE,
    dragStartPoint,
    currentTranslateX,
    touchMoved = false,
    numItems = 0,
    peekModeNumItems = 0,
    headText,
    subheadText,
    currentPeekIndex,
    previousPeekIndex,
    elements,
    self;

function getMenuXTranslation() {
    if (menuCurrentState === state.PEEK) {
        return PEEK_MODE_TRANSLATE_X;
    }
    if (menuCurrentState === state.VISIBLE) {
        return FULL_MENU_TRANSLATE_X;
    }
    return HIDDEN_MENU_TRANSLATE_X;
}

function positionHandle() {
    var handle = document.getElementById('contextMenuHandle'),
        top,
        moreIcon = document.getElementById('moreHandleIcon');

    if (menuCurrentState === state.PEEK) {
        handle.className = 'showContextMenuHandle';
        top = (window.screen.availHeight + peekModeNumItems * 121) / 2;
        handle.style.top = top + 'px';

        // If have more options than the limit, show the more dots on the handle
        if (numItems > MAX_NUM_ITEMS_IN_PEEK_MODE && moreIcon == null) {
            moreIcon = document.createElement('img');
            moreIcon.id = "moreHandleIcon";
            moreIcon.style = 'showMoreHandleIcon';
            moreIcon.src = 'assets/ActionOverflowMenu.png';
            moreIcon.className = 'showMoreHandleIcon';
            handle.appendChild(moreIcon);
        } else if (numItems < MAX_NUM_ITEMS_IN_PEEK_MODE && moreIcon != null) {
            handle.removeChild(moreIcon);
        }

    } else if (menuCurrentState === state.VISIBLE) {
        if (numItems <= MAX_NUM_ITEMS_IN_PEEK_MODE) {
            handle.className = 'showContextMenuHandle';
            top = (window.screen.availHeight + numItems * 121) / 2;
            handle.style.top = top + 'px';
        } else {
            handle.className = 'hideContextMenuItem';
        }
    }
}

function menuDragStart() {
    menu.style.webkitTransitionDuration = '0s';
    menu.style.overflowX = 'hidden';
    menu.style.overflowY = 'scroll';
}

function menuDragMove(pageX) {
    var x = window.screen.width + getMenuXTranslation() + pageX - dragStartPoint,
        menuWidth = -FULL_MENU_TRANSLATE_X;
    // Stop translating if the full menu is on the screen
    if (x >= window.screen.width - menuWidth) {
        currentTranslateX = getMenuXTranslation() + pageX - dragStartPoint;
        menu.style.webkitTransform = 'translate(' + currentTranslateX + 'px' + ', 0)';
    }
}

function menuDragEnd() {
    menu.style.webkitTransitionDuration = '0.25s';

    menuCurrentState = state.DRAGEND;
    if (currentTranslateX > PEEK_MODE_TRANSLATE_X) {
        self.hideContextMenu();
    } else if (currentTranslateX < FULL_MENU_TRANSLATE_X / 2) {
        self.showContextMenu();
    } else {
        self.peekContextMenu();
    }
    
    menu.style.webkitTransform = '';
}

function menuTouchStartHandler(evt) {
    evt.stopPropagation();
    menuDragStart();
    dragStartPoint = evt.touches[0].pageX;
}

function bodyTouchStartHandler(evt) {
    dragStartPoint = evt.touches[0].pageX;
    menuDragStart();
}

function menuTouchMoveHandler(evt) {
    evt.stopPropagation();
    touchMoved = true;
    menuDragMove(evt.touches[0].pageX);
}

function bodyTouchMoveHandler(evt) {
    touchMoved = true;
    menuDragMove(evt.touches[0].pageX);
}

function menuTouchEndHandler(evt) {
    evt.stopPropagation();
    if (touchMoved) {
        touchMoved = false;
        menuDragEnd();
    } else {
        if (menuCurrentState === state.PEEK) {
            self.showContextMenu();
        } else if (menuCurrentState === state.VISIBLE) {
            self.peekContextMenu();
        }
    }
}

function bodyTouchEndHandler(evt) {
    if (touchMoved) {
        touchMoved = false;
        menuDragEnd();
    }
    else {
        self.hideContextMenu();
    }
}

function getMenuItemAtPosition(currentYPosition, elementHeight) {
    var screenHeight = window.screen.availHeight,
        diff = currentYPosition - screenHeight / 2,
        elementIndex,
        numOfItems = 0;
    if (menuCurrentState === state.PEEK) {
        numOfItems = peekModeNumItems;
    } else if (menuCurrentState === state.VISIBLE) {
        numOfItems = numItems;
    }
    // Base case that we have just a single one, so index that one on touchend
    if (numOfItems === 1) {
        elementIndex = 0;
    } else {
        elementIndex = (numOfItems >> 1) + (diff / elementHeight) | 0;
    }

    //Check if the index is greater than the number of elems or less than
    //if so, let's reset the elements and hide all elements
    if (elementIndex >= numOfItems || elementIndex < 0) {
        elementIndex = -1;
    }
    return elementIndex;
}

function highlightMenuItem(item) {
    if (menuCurrentState === state.PEEK) {
        item.className = 'contextmenuItem showContextmenuItem';
        item.active = true;
    } else if (menuCurrentState === state.VISIBLE) {
        item.className = 'contextmenuItem fullContextmenuItem';
        item.active = true;
    }
}

function menuItemTouchStartHandler(evt) {
    evt.stopPropagation();
    highlightMenuItem(evt.currentTarget);
    previousPeekIndex = currentPeekIndex = evt.currentTarget.index;
}

function menuItemTouchMoveHandler(evt) {
    evt.stopPropagation();
    var currentYPosition = evt.touches[0].clientY,
        elementHeight = evt.currentTarget.clientHeight + 2; // border = 2
    currentPeekIndex = getMenuItemAtPosition(currentYPosition, elementHeight);
    if (currentPeekIndex === previousPeekIndex) {
        return;
    }
    if (currentPeekIndex === -1) {
        if (elements[previousPeekIndex].active) {
            elements[previousPeekIndex].className = 'contextmenuItem';
            elements[previousPeekIndex].active = false;
        }
    } else if (previousPeekIndex === -1) {
        highlightMenuItem(elements[currentPeekIndex]);
    } else {
        if (elements[previousPeekIndex].active) {
            elements[previousPeekIndex].className = 'contextmenuItem';
            elements[previousPeekIndex].active = false;
        }
        highlightMenuItem(elements[currentPeekIndex]);
    }
    previousPeekIndex = currentPeekIndex;
}

function menuItemTouchEndHandler(evt) {
    evt.stopPropagation();
    if (currentPeekIndex !== -1) {
        var element = elements[currentPeekIndex];
        element.className = 'contextmenuItem';
        element.active = false;
        window.qnx.webplatform.getController().remoteExec(1, 'executeMenuAction', [element.attributes.actionId.value]);
        self.hideContextMenu();
    }
}

function rotationHandler() {
    if (window.orientation === 0 || window.orientation === 180) {
        MAX_NUM_ITEMS_IN_PEEK_MODE = MAX_NUM_ITEMS_IN_PORTRAIT_PEEK_MODE;
    } else {
        MAX_NUM_ITEMS_IN_PEEK_MODE = MAX_NUM_ITEMS_IN_LANDSCAPE_PEEK_MODE;
    }
    self.hideContextMenu();
}

function handleMouseDown(evt) {
    evt.preventDefault();
    evt.stopPropagation();
}

function setHeadText(text) {
    var headText = document.getElementById('contextMenuHeadText');
    headText.innerText = text;
    if (text) {
        headText.style.height = '60px';
    } else {
        headText.style.height = '0px';
    }
}

function setSubheadText(text) {
    var subheadText = document.getElementById('contextMenuSubheadText');
    subheadText.innerText = text;
    if (text) {
        subheadText.style.height = '60px';
    } else {
        subheadText.style.height = '0px';
    }
}

function init() {
    menu = document.getElementById('contextMenu');
    menu.addEventListener('webkitTransitionEnd', self.transitionEnd.bind(self));
    menu.addEventListener('touchstart', menuTouchStartHandler);
    menu.addEventListener('touchmove', menuTouchMoveHandler);
    menu.addEventListener('touchend', menuTouchEndHandler);
    setHeadText('');
    setSubheadText('');
    rotationHandler();
    window.addEventListener('orientationchange', rotationHandler, false);
}

self = {
    init: init,
    handleMouseDown: handleMouseDown,
    setMenuOptions: function (options) {
        var menuContent = document.getElementById("contextMenuContent"),
            menuItem,
            menuImage,
            i,
            option;

        for (i = 0; i < options.length; i++) {
            option = options[i];
            if (option.headText || option.subheadText) {
                if (option.headText) {
                    headText = option.headText;
                }
                if (option.subheadText) {
                    subheadText = option.subheadText;
                }
                continue;
            }
            menuItem = document.createElement('div');

            menuImage = document.createElement('img');
            menuImage.src = option.imageUrl ? option.imageUrl : 'assets/generic_81_81_placeholder.png';
            menuItem.appendChild(menuImage);
            menuItem.appendChild(document.createTextNode(option.label));
            menuItem.setAttribute("class", "contextmenuItem");
            if (numItems >= MAX_NUM_ITEMS_IN_PEEK_MODE) {
                menuItem.setAttribute('class', 'hideContextMenuItem');
            }
            menuItem.setAttribute("actionId", option.actionId);
            menuItem.index = numItems;
            menuItem.active = false;
            menuItem.addEventListener('mousedown', handleMouseDown, false);
            menuItem.addEventListener('touchstart', menuItemTouchStartHandler);
            menuItem.addEventListener('touchmove', menuItemTouchMoveHandler);
            menuItem.addEventListener('touchend', menuItemTouchEndHandler);
            menuContent.appendChild(menuItem);
            numItems++;
        }
    },

    showContextMenu: function (evt) {
        var i,
            header,
            menuContent,
            item;

        if (menuCurrentState === state.VISIBLE) {
            return;
        }
        menu.style.webkitTransitionDuration = '0.25s';
        menu.className = 'showContextMenu';
        document.getElementById('contextMenuContent').className = 'contentShown';
        document.getElementById('contextMenuHandle').className = 'showContextMenuHandle';

        if (evt) {
            evt.preventDefault();
            evt.stopPropagation();
        }

        // Show header
        if (headText || subheadText) {
            header = document.getElementById('contextMenuHeader');
            header.className = 'showMenuHeader';
            if (headText) {
                setHeadText(headText);
            }
            if (subheadText) {
                setSubheadText(subheadText);
            }
            // Move content so that menu items won't be covered by header
            if (numItems > MAX_NUM_ITEMS_IN_PEEK_MODE) {
                menuContent = document.getElementById('contextMenuContent');
                menuContent.style.position = 'absolute';
                menuContent.style.top = '131px';
            }
        }
        
        // Show all menu items
        if (numItems > MAX_NUM_ITEMS_IN_PEEK_MODE) {
            item = document.getElementById('contextMenuContent').firstChild;
            for (i = 1; i < numItems; i++) {
                item = item.nextSibling;
                if (i < MAX_NUM_ITEMS_IN_PEEK_MODE) {
                    continue;
                }
                item.className = 'contextmenuItem';
            }
        }
        menuCurrentState = state.VISIBLE;
        positionHandle();
    },

    isMenuVisible: function () {
        return menuCurrentState === state.PEEK || menuCurrentState === state.VISIBLE;
    },

    hideContextMenu: function (evt) {
        if (menuCurrentState === state.HIDE) {
            return;
        }
        numItems = 0;
        menu.style.webkitTransitionDuration = '0.25s';
        menu.className = 'hideMenu';

        menu.removeEventListener('touchstart', menuTouchStartHandler, false);
        menu.removeEventListener('touchmove', menuTouchMoveHandler, false);
        menu.removeEventListener('touchend', menuTouchEndHandler, false);

        window.document.body.removeEventListener('touchstart', bodyTouchStartHandler, false);
        window.document.body.removeEventListener('touchmove', bodyTouchMoveHandler, false);
        window.document.body.removeEventListener('touchend', bodyTouchEndHandler, false);

        var menuContent = document.getElementById('contextMenuContent');
        while (menuContent.firstChild) {
            menuContent.removeChild(menuContent.firstChild);
        }
        menuContent.style.position = '';
        menuContent.style.top = '';

        window.qnx.webplatform.getController().remoteExec(1, 'webview.notifyContextMenuCancelled');
        if (evt) {
            evt.preventDefault();
            evt.stopPropagation();
        }
        menuCurrentState = state.HIDE;
        // Reset sensitivity
        window.qnx.webplatform.getController().remoteExec(1, 'webview.setSensitivity', ['SensitivityTest']);
    },

    setHeadText: setHeadText,

    setSubheadText: setSubheadText,

    peekContextMenu: function (show, zIndex) {
        if (menuCurrentState === state.PEEK) {
            return;
        }
        peekModeNumItems = numItems > MAX_NUM_ITEMS_IN_PEEK_MODE ? MAX_NUM_ITEMS_IN_PEEK_MODE : numItems;
        elements = document.getElementsByClassName("contextmenuItem");
        var menuContent = document.getElementById('contextMenuContent'),
            item,
            i,
            header;

        // Cache items for single item peek mode.
        window.qnx.webplatform.getController().remoteExec(1, 'webview.setSensitivity', ['SensitivityAlways']);
        menu.style.webkitTransitionDuration = '0.25s';
        menu.className = 'peekContextMenu';
        document.getElementById('contextMenuHandle').className = 'showContextMenuHandle';
        if ((menuCurrentState === state.DRAGEND || menuCurrentState === state.VISIBLE)) {
            // Hide some items if numItems > MAX_NUM_ITEMS_IN_PEEK_MODE
            if (numItems > MAX_NUM_ITEMS_IN_PEEK_MODE) {
                item = document.getElementById('contextMenuContent').firstChild;
                for (i = 1; i < numItems; i++) {
                    item = item.nextSibling;
                    if (i < MAX_NUM_ITEMS_IN_PEEK_MODE) {
                        continue;
                    }
                    item.className = 'hideContextMenuItem';
                }
            }
        }

        // Always hide the header div whenever we are peeking
        if (headText || subheadText) {
            header = document.getElementById('contextMenuHeader');
            header.className = '';
            if (headText) {
                setHeadText('');
            }
            if (subheadText) {
                setSubheadText('');
            }
            if (numItems > MAX_NUM_ITEMS_IN_PEEK_MODE) {
                menuContent.style.position = '';
                menuContent.style.top = '';
            }
        }

        // This is for single item peek mode
        menu.style.overflowX = 'visible';
        menu.style.overflowY = 'visible';

        window.document.body.addEventListener('touchstart', bodyTouchStartHandler);
        window.document.body.addEventListener('touchmove', bodyTouchMoveHandler);
        window.document.body.addEventListener('touchend', bodyTouchEndHandler);

        menuCurrentState = state.PEEK;
        positionHandle();
    },

    transitionEnd: function () {
        if (menuCurrentState === state.HIDE) {
            self.setHeadText('');
            self.setSubheadText('');
            headText = '';
            subheadText = '';
        }
    }
};

module.exports = self;
