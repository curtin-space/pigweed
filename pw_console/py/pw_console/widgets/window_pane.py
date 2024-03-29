# Copyright 2021 The Pigweed Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
"""Window pane base class."""

from abc import ABC
from typing import Optional, TYPE_CHECKING
import functools

from prompt_toolkit.layout.dimension import AnyDimension

from prompt_toolkit.filters import Condition
from prompt_toolkit.layout import (
    ConditionalContainer,
    Dimension,
    HSplit,
)

from pw_console.get_pw_console_app import get_pw_console_app

import pw_console.widgets.checkbox
import pw_console.widgets.mouse_handlers
import pw_console.style

if TYPE_CHECKING:
    from pw_console.console_app import ConsoleApp


class WindowPaneHSplit(HSplit):
    """PromptToolkit HSplit that saves the current width and height.

    This overrides the write_to_screen function to save the width and height of
    the container to be rendered.
    """
    def __init__(self, parent_window_pane, *args, **kwargs):
        # Save a reference to the parent window pane.
        self.parent_window_pane = parent_window_pane
        super().__init__(*args, **kwargs)

    def write_to_screen(
        self,
        screen,
        mouse_handlers,
        write_position,
        parent_style: str,
        erase_bg: bool,
        z_index: Optional[int],
    ) -> None:
        # Save the width and height for the current render pass. This will be
        # used by the log pane to render the correct amount of log lines.
        self.parent_window_pane.update_pane_size(write_position.width,
                                                 write_position.height)
        # Continue writing content to the screen.
        super().write_to_screen(screen, mouse_handlers, write_position,
                                parent_style, erase_bg, z_index)


class WindowPane(ABC):
    """The Pigweed Console Window Pane parent class."""

    # pylint: disable=too-many-instance-attributes
    def __init__(
        self,
        application: Optional['ConsoleApp'] = None,
        pane_title: str = 'Window',
        height: Optional[AnyDimension] = None,
        width: Optional[AnyDimension] = None,
    ):
        if application:
            self.application = application
        else:
            self.application = get_pw_console_app()

        self._pane_title = pane_title
        self._pane_subtitle = None

        # Default width and height to 50% of the screen
        self.height = height if height else Dimension(weight=50)
        self.width = width if width else Dimension(weight=50)

        # Boolean to show or hide this window pane
        self.show_pane = True
        # Booleans for toggling top and bottom toolbars
        self.show_top_toolbar = True
        self.show_bottom_toolbar = True

        # Height and width values for the current rendering pass.
        self.current_pane_width = 0
        self.current_pane_height = 0
        self.last_pane_width = 0
        self.last_pane_height = 0

    def pane_title(self) -> str:
        return self._pane_title

    def set_pane_title(self, title: str) -> None:
        self._pane_title = title

    def menu_title(self) -> str:
        """Return a title to display in the Window menu."""
        return self.pane_title()

    def pane_subtitle(self) -> str:  # pylint: disable=no-self-use
        """Further title info for display in the Window menu."""
        return ''

    def redraw_ui(self) -> None:
        """Redraw the prompt_toolkit UI."""
        if not hasattr(self, 'application'):
            return
        # Thread safe way of sending a repaint trigger to the input event loop.
        self.application.redraw_ui()

    def focus_self(self) -> None:
        """Switch prompt_toolkit focus to this window pane."""
        if not hasattr(self, 'application'):
            return
        self.application.focus_on_container(self)

    def __pt_container__(self):
        """Return the prompt_toolkit root container for this log pane.

        This allows self to be used wherever prompt_toolkit expects a container
        object."""
        return self.container  # pylint: disable=no-member

    def get_all_key_bindings(self) -> list:
        """Return keybinds for display in the help window.

        For example:

        Using a prompt_toolkit control:

          return [self.some_content_control_instance.get_key_bindings()]

        Hand-crafted bindings for display in the HelpWindow:

          return [{
              'Execute code': ['Enter', 'Option-Enter', 'Meta-Enter'],
              'Reverse search history': ['Ctrl-R'],
              'Erase input buffer.': ['Ctrl-C'],
              'Show settings.': ['F2'],
              'Show history.': ['F3'],
          }]
        """
        # pylint: disable=no-self-use
        return []

    def get_all_menu_options(self) -> list:
        """Return menu options for the window pane.

        Should return a list of tuples containing with the display text and
        callable to invoke on click.
        """
        # pylint: disable=no-self-use
        return []

    def update_pane_size(self, width, height) -> None:
        """Save pane width and height for the current UI render pass."""
        if width:
            self.last_pane_width = self.current_pane_width
            self.current_pane_width = width
        if height:
            self.last_pane_height = self.current_pane_height
            self.current_pane_height = height

    def _create_pane_container(self, *content) -> ConditionalContainer:
        return ConditionalContainer(
            WindowPaneHSplit(
                self,
                content,
                # Window pane dimensions
                height=lambda: self.height,
                width=lambda: self.width,
                style=functools.partial(pw_console.style.get_pane_style, self),
            ),
            filter=Condition(lambda: self.show_pane))
