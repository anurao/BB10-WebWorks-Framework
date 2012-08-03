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

var MAX_NUM_ITEMS_IN_PEEK_MODE = 7,
    PEEK_MODE_TRANSLATE_X = -121,
    FULL_MENU_TRANSLATE_X = -569,
    HIDDEN_MENU_TRANSLATE_X = 0,
    state = {
        HIDE: 0,
        PEEK: 1,
        VISIBLE: 2,
        DRAGEND: 3
    },
    menu = document.getElementById('contextMenu'),
    contextmenu,
    menuCurrentState = state.HIDE,
    height,     // FIXME: remove this when 186908 fixed
    dragStartPoint,
    currentTranslateX,
    touchMoved = false,
    numItems = 0,
    peekModeNumItems = 0,
    headText,
    subheadText,
    currentContext,
    config,
    utils,
    includePath,
    elementToExecute,
    previousIndex,
    elements,
    elementToExecute,
    elementsLength;

function requireLocal(id) {
    return require(!!require.resolve ? "../../" + id.replace(/\/chrome/, "") : id);
}

function getMenuXTranslation() {
    if (menuCurrentState === state.PEEK) {
        return PEEK_MODE_TRANSLATE_X;
    }
    if (menuCurrentState === state.VISIBLE) {
        return FULL_MENU_TRANSLATE_X;
    }
    return HIDDEN_MENU_TRANSLATE_X;
}

function menuTouchStartHandler(evt) {
    menu.style.webkitTransitionDuration = '0s';
    menu.style.overflowX = 'hidden';
    menu.style.overflowY = 'scroll';
    dragStartPoint = evt.touches[0].pageX;
    evt.stopPropagation();
}

function menuTouchMoveHandler(evt) {
    var touch = evt.touches[0],
        x = window.screen.width + getMenuXTranslation() + touch.pageX - dragStartPoint,
        menuWidth = -FULL_MENU_TRANSLATE_X;

    touchMoved = true;
    // Stop translating if the full menu is on the screen
    if (x >= window.screen.width - menuWidth) {
        currentTranslateX = getMenuXTranslation() + touch.pageX - dragStartPoint;
        menu.style.webkitTransform = 'translate(' + currentTranslateX + 'px' + ', 0)';
    }
}

function menuTouchEndHandler(evt) {
    if (touchMoved) {
        touchMoved = false;
        menuCurrentState = state.DRAGEND;
        if (currentTranslateX > PEEK_MODE_TRANSLATE_X) {
            contextmenu.hideContextMenu();
        } else if (currentTranslateX < FULL_MENU_TRANSLATE_X / 2) {
            contextmenu.showContextMenu();
        } else {
            contextmenu.peekContextMenu();
        }
        menu.style.webkitTransform = '';
    } else {
        if (menuCurrentState === state.PEEK) {
            contextmenu.showContextMenu();
        } else if (menuCurrentState === state.VISIBLE) {
            contextmenu.peekContextMenu();
        }
    }
}


function getNewElementSelection(currentYPosition, elementHeight) {
    var screenHeight = window.screen.availHeight,
        middle = screenHeight / 2,
        diff = currentYPosition - middle,
        elementIndex;

    // Base case that we have just a single one, so index that one on touchend
    if (elementsLength === 1) {
        elementIndex = 0;
    }
    else {
        elementIndex = (elementsLength >> 1) + (diff / elementHeight) | 0;
    }

    //Check if the index is greater then the number of elems or less then
    //if so, let's reset the elements and hide all elements
    if (elementIndex > elementsLength || elementIndex < 0) {
        previousIndex = null;
        elementToExecute = null;
        return;
    }

    // Check if we are still on the same element? then return nothing
    // or if we somehow get an off calculation don't do anything
    if (previousIndex === elementIndex) {
        return;
    }

    // Else continue and set our values for next time
    // while returning the new element to be shown
    previousIndex = elementIndex;
    elementToExecute = elements[elementIndex];
    return elementToExecute;
}

function menuItemTouchStartHandler(evt) {
    evt.stopPropagation();
    if (menuCurrentState === state.PEEK) {
        evt.currentTarget.className = 'menuItem showItem';
    } else {
        evt.currentTarget.className = 'menuItem highlightItem';
    }
    elementToExecute = evt.currentTarget;
    getNewElementSelection(evt.touches[0].clientY, evt.currentTarget.clientHeight);
}

function menuItemTouchMoveHandler(touchEvent) {
    touchEvent.stopPropagation();

    var currentYPosition = touchEvent.touches[0].clientY,
        elementHeight = touchEvent.currentTarget.clientHeight,
        elementToShow,
        previousToHideIndex = previousIndex;

    //Base on our current Y position let's calculate which
    // element this touch point belongs to
    elementToShow = getNewElementSelection(currentYPosition, elementHeight);

    if (elementToShow) {
        if ((typeof previousToHideIndex !== "undefined") && elements[previousToHideIndex]) {
            elements[previousToHideIndex].className = "menuItem peekItem";
        }
        if (menuCurrentState === state.PEEK) {
            elementToShow.className = 'menuItem showItem';
        } else if (menuCurrentState === state.VISIBLE) {
            elementToShow.className = 'menuItem highlightItem';
        }
    } else if (!elementToExecute) {
        if ((typeof previousIndex !== 'undefined') && elements[previousIndex]) {
            elements[previousIndex].className = 'menuItem peekItem';
            previousIndex = null;
        }
    }
}

function menuItemTouchEndActionInvoker(actionId, menuItem) {
    if (menuItem) {
        menuItem.className = 'menuItem peekItem';
    }
    if (elementToExecute) {
        window.qnx.webplatform.getController().remoteExec(1, 'executeMenuAction', [elementToExecute.attributes.actionId.value]);
    } else {
        window.qnx.webplatform.getController().remoteExec(1, 'executeMenuAction', [actionId]);
    }

    contextmenu.hideContextMenu();

    elementToExecute = null;
    previousIndex = null;
}

function menuItemTouchEndHandler(touchEvent) {
    touchEvent.stopPropagation();
}

function init() {
    config = requireLocal("../chrome/lib/config.js");
    utils = requireLocal("../chrome/lib/utils");

    menu.addEventListener('webkitTransitionEnd', contextmenu.transitionEnd.bind(contextmenu));
    menu.addEventListener('touchstart', menuTouchStartHandler);
    menu.addEventListener('touchmove', menuTouchMoveHandler);
    menu.addEventListener('touchend', menuTouchEndHandler);
}

contextmenu = {
    init: init,
    setMenuOptions: function (options) {
        var menuContent = document.getElementById("contextMenuContent"),
            header,
            menuItem,
            menuImage,
            i;

        while (menuContent.childNodes.length >= 1) {
            menuContent.removeChild(menu.firstChild);
        }
        contextmenu.setHeadText('');
        contextmenu.setSubheadText('');

        for (i = 0; i < options.length; i++) {
            if (options[i].headText || options[i].subheadText) {
                header = document.getElementById('contextMenuHeader');
                header.className = 'contextMenuHeader';
                if (options[i].headText) {
                    contextmenu.setHeadText(options[i].headText);
                }
                if (options[i].subheadText) {
                    contextmenu.setSubheadText(options[i].subheadText);
                }
            } else {
                menuItem = document.createElement('div');
                
                menuImage = document.createElement('img');
                menuImage.src = options[i].imageUrl ? options[i].imageUrl : 'assets/generic_81_81_placeholder.png';
               
                menuItem.appendChild(menuImage);
                menuItem.appendChild(document.createTextNode(options[i].label));
                menuItem.setAttribute("class", "menuItem");
                menuItem.setAttribute("actionId", options[i].actionId);
                menuItem.addEventListener('mousedown', contextmenu.handleMouseDown, false);
                menuItem.addEventListener('touchstart', menuItemTouchStartHandler);
                menuItem.addEventListener('touchmove', menuItemTouchMoveHandler);
                menuItem.addEventListener('touchend', menuItemTouchEndHandler);
                menuItem.ontouchend = menuItemTouchEndActionInvoker.bind(this, options[i].actionId, menuItem);

                menu.appendChild(menuItem);
                numItems++;
            }
        }
    },

    handleMouseDown: function (evt) {
        evt.preventDefault();
        evt.stopPropagation();
    },

    setHeadText: function (text) {
        var headText = document.getElementById('contextMenuHeadText');
        headText.innerText = text;
        if (text) {
            headText.style.height = '60px';
        } else {
            headText.style.height = '0px';
        }
    },

    setSubheadText: function (text) {
        var subheadText = document.getElementById('contextMenuSubheadText');
        subheadText.innerText = text;
        if (text) {
            subheadText.style.height = '60px';
        } else {
            subheadText.style.height = '0px';
        }
    },

    showContextMenu: function (evt) {
        if (menuCurrentState === state.VISIBLE) {
            return;
        }

        var menuContent = document.getElementById('contextMenuContent'),
            handle = document.getElementById('contextMenuHandle'),
            i;


        menu.className = 'showMenu';
        menuContent.className = 'contentShown';
        handle.className = 'showContextMenuHandle';

        if (evt) {
            evt.preventDefault();
            evt.stopPropagation();
        }

        if (menuCurrentState === state.PEEKED) {
            evt.cancelBubble = true;
        }

        // reset the event listeners since we are now shown, and do not
        // want to continue unfolding the menu
        for (i = 0; i < elements.length; i++) {
            elements[i].ontouchmove = null;
        }

        menuCurrentState = state.VISIBLE;

    },

    isMenuVisible: function () {
        return menuCurrentState === state.PEEK || menuCurrentState === state.VISIBLE;
    },

    hideContextMenu: function (evt) {
        if (menuCurrentState === state.HIDE) {
            return;
        }

        var contextMenuContent = document.getElementById('contextMenuContent'),
            handle = document.getElementById('contextMenuHandle'),
            elements = document.getElementsByClassName("menuItem"),
            i;

        numItems = 0;

        menu.removeEventListener('touchend', contextmenu.hideContextMenu, false);
        handle.removeEventListener('touchend', contextmenu.showContextMenu, false);

        menu.className = 'hideMenu';
        window.qnx.webplatform.getController().remoteExec(1, 'webview.notifyContextMenuCancelled');

        if (evt) {
            evt.preventDefault();
            evt.stopPropagation();
        }

        for (i = 0; i < elements.length; i++) {
            elements[i].className = "menuItem";
        }

        // Reset the scrolling of any divs in the menu, since it will save the scroll
        contextMenuContent.scrollTop = 0;

        // Reset sensitivity
        window.qnx.webplatform.getController().remoteExec(1, 'webview.setSensitivity', ['SensitivityTest']);

        menuCurrentState = state.HIDE;
    },

    peekContextMenu: function (show, zIndex) {
        if (menuCurrentState === state.PEEK) {
            return;
        }

        var handle = document.getElementById('contextMenuHandle'),
            menuContent = document.getElementById('contextMenuContent'),
            menuItems = document.getElementsByClassName('menuItem');

        if (menuItems.length > 7) {
            handle.className = 'showMoreActionsHandle';
        } else {
            handle.className = 'showContextMenuHandle';
        }

        peekModeNumItems = numItems > MAX_NUM_ITEMS_IN_PEEK_MODE ? MAX_NUM_ITEMS_IN_PEEK_MODE : numItems;
        // Cache items for single item peek mode.
        elements = document.getElementsByClassName("contextmenuItem");
        window.qnx.webplatform.getController().remoteExec(1, 'webview.setSensitivity', ['SensitivityAlways']);

        elements = menuItems;
        elementsLength = elements.length;

        menuContent.className = 'contentPeeked';
        menu.className = 'peekContextMenu';

        // This is for single item peek mode
        menu.style.overflowX = 'visible';
        menu.style.overflowY = 'visible';

        menuCurrentState = state.PEEK;
    },

    transitionEnd: function () {
        var handle = document.getElementById('contextMenuHandle'),
            header;
        if (menuCurrentState === state.VISIBLE) {
            menu.addEventListener('touchend', contextmenu.hideContextMenu, false);
            handle.removeEventListener('touchend', contextmenu.showContextMenu, false);
        } else if (menuCurrentState === state.PEEKED) {
            handle.addEventListener('touchend', contextmenu.showContextMenu, false);
            menu.addEventListener('touchend', contextmenu.hideContextMenu, false);
        } else {
            header = document.getElementById('contextMenuHeader');
            header.className = 'contextMenuHeaderEmpty';
            contextmenu.setHeadText('');
            contextmenu.setSubheadText('');
        }
    },

    setCurrentContext: function (context) {
        currentContext = context;
    }
};

module.exports = contextmenu;
