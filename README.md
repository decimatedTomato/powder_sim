## CONTROLS
- `f` fullscreen/windowed mode
- `p` pause/unpause
- `.` step 1 update
- `r` reset to saved state

## PURPOSE
Cellular Automata Particle simulation inspired by
- https://www.youtube.com/watch?v=prXuyMCgbTc&ab_channel=GDC
- https://dan-ball.jp/en/javagame/dust/

Makes use of:
- glfw for window context
- OpenGL for rendering

## PROBLEMS
- This one somehow fixed itself
<!-- - Simulation exhibits strong bias
- Bias is flipped depending on whether left-most or right-most columns are evaluated first
- Not sure how to handle the order of evaluation of cells
    - I thought about  -->
- If a particle has negative gravity (eg. smoke rising) a double buffered grid is needed
    - Otherwise as the rows get evaluated bottom to top, the particles would teleport to the top in only one update
- Additional grids could be needed used for things like particle velocities, that are independent from the particle type
- At that point a particle type ID might be better than the full-color value, although for now the color is fine
- Multiple particle type IDs could be powers of 2, so that they can be used as a bitmask for elegant interactions

## IDEAS
- Change particle model to type_id rather than color
- Give particles additional properties such as
    - flammability
    - mass (displacement, gravity direction)
    - Stasis (true / false)
    - Emissivity

- Add non-simulated particles for effect like splashes
- Add gravity and momentum into simulation
- Write instructions into stdout
- Keeping record of past frames
    - Export video
    - Step backwards
- Export frames as images
- Import images as frame
- Save and load state of grid

- Add mouse based particle brush
- Add particle brush type and size input