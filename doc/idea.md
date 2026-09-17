# World clock QT application

A QT GUI app that would allow a user to have multiple clock widgets for different time zones.

An app would start with one clock widget showing user's local time.
Then a user would be able to edit the layout adding clocks to the right/left or top/bottom from the existing clock.
A user should be able to drag/move the clock to a different position in the edit mode.

Each clock must have a caption underneath the clock widget showing the timezone/Location, eg "London", "New York", "Geneva", etc.
When addig a new clock a user would be prompted to select the location or a time zone for the clock being added.
A user may chose to remove a clock from the panel. Except for the last clock, I guess it makes no sense to remove the last clock on the panel.

The configuration must be stored in the user's home directory, in a toml config file: ~/.config/qworldclock/qworldclock.cfg


## spec
Language: C++
Target platforms: Linux, Windows
Build system: cmake, pixi

## widget sizes
We have two options here: either let the user resize the window and resize the widgets keeping the geometry,
or to have a fixed sized widgets and just let the user to set the clock sizes.
I think we should support both modes. The font size of the caption underneath the clock must adapt accordingly.
