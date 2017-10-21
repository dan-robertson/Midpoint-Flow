# Midpoint flow

Consider a polygon and replace each line segment by its midpoint to
make a new polygon. The new polygon will be slightly smaller so
normalise it to fit inside a fixed size rectangle. It turns out that
when iterating this process the polygon will approach an ellipse in
shape. This program exhibits this.

# Curvature flow

There is also some code to attempt some kind of curvature flow, that
is estimate the curvature at each point and try to move each point
towards the centre of its circle of curvature. This does not seem so
effective.

# Instructions

To build:

    $ make

To run:

	$ ./pe

This depends on SDL2.
