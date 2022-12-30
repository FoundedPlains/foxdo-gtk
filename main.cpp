/*
foxdo-gtk - GTK frontend for foxdo
Copyright (C) 2022 Ján Gajdoš

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

extern "C" {
    #include "foxstd.h"
    #include "foxdo/src/rootize.h"
    #include "foxdo/src/launch.h"
    #include "foxdo/src/timeout.h"
    #include "foxdo/src/config_parser.h"
    #include "foxdo/src/check.h"
}

#include <unistd.h>

#include <gtkmm.h>
#include <gdkmm.h>

void error_msg() {
    Gtk::MessageDialog messageBox("Wrong Password", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
    messageBox.set_modal();
    messageBox.set_position(Gtk::WindowPosition::WIN_POS_CENTER);
    messageBox.run();
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("foxdo-gtk [PROG] [ARGS]\n");
        return EINVAL;
    }

    FILE *conf = fopen("/etc/foxdo.foxconfig", "r");

    if (!conf)
    {
        printf("/etc/foxdo.foxconfig doesn't exist!\n");
        return ENOENT;
    }

    Config config = parseConfig();

    short has_group = check_groups(config);
    short has_user = check_user(config);

    char *login = getenv("USER");

    if (!(has_group || has_user))
    {
        printf("You are not allowed to use foxdo-gtk!\n");
        return EACCES;
    }

    short result = check_self(login, time(NULL));

    //! this might create security holes

    rootize();

    int heh = 0;
    char **fargs = (char**) malloc(sizeof(char*));

    char *args[argc - 1];

    for (int i = 1; i < argc; i++) {
        args[i - 1] = argv[i];
    }

    args[argc - 1] = NULL;

    if (argc == 2) {
        if (!result)
            launchWithoutArgs(argv[1]);
    } else
        if (!result)
            launchWithArgs(argv[1], args);

    auto app = Gtk::Application::create(heh, fargs, "org.firethefox.foxdo-gtk");

    Gtk::Window window;
    Gtk::Grid grid;
    Gtk::Box box;
    Gtk::Image icon;
    Gtk::Label text;
    Gtk::Entry passwd;
    Gtk::Button accept;

    icon.set_from_icon_name("dialog-password", Gtk::ICON_SIZE_MENU);
    icon.set_pixel_size(64);

    passwd.set_visibility(false);
    accept.set_label("Accept");
    accept.signal_button_release_event().connect([&](GdkEventButton*) {
        char *pass = (char *) passwd.get_text().c_str();

        if (argc == 2) {
            if (result && !FoxAuth_checkPassword(login, pass)) {
                if (result)
                    register_self(login, time(NULL) + config.t.seconds + (config.t.minutes * 60) + (config.t.hours * 60 * 60));
                window.close();

                launchWithoutArgs(argv[1]);
            } else {
                if (!result) {
                    window.close();

                    launchWithoutArgs(argv[1]);
                } else {
                    error_msg();
                }
            }
            return true;
        }

        if (result && !FoxAuth_checkPassword(login, pass)) {
            if (result)
                register_self(login, time(NULL) + config.t.seconds + (config.t.minutes * 60) + (config.t.hours * 60 * 60));

            launchWithArgs(argv[1], args);
        } else {
            if (!result) {
                launchWithArgs(argv[1], args);
            } else {
                error_msg();
            }
        }
        return true;
    });

    text.set_markup("<span size=\"large\">Enter password</span>");

    box.set_homogeneous(false);
    box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);

    box.add(icon);
    box.add(text);

    grid.attach(box, 0, 0, 3, 1);
    grid.attach(passwd, 0, 1, 2, 1);
    grid.attach(accept, 2, 1, 1, 1);

    grid.set_column_homogeneous(true);

    icon.set_halign(Gtk::ALIGN_START);
    text.set_halign(Gtk::ALIGN_START);

    window.add(grid);
    grid.show();
    grid.show_all_children();

    grid.set_margin_top(15);
    grid.set_margin_start(15);
    grid.set_margin_end(15);
    grid.set_margin_bottom(15);

    grid.set_column_spacing(15);

    window.set_decorated(false);
    window.set_position(Gtk::WindowPosition::WIN_POS_CENTER);
    window.set_resizable(false);
    window.set_size_request(350);

    return app->run(window);
}