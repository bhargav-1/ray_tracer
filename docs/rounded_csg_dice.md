# Rounded CSG dice (Chapter 16)

Open [rounded_csg_dice_chapter16.yaml](../assets/scenes/rounded_csg_dice_chapter16.yaml) in the scene editor.

Each die uses a CSG `difference`: the left operand is the colored rounded body,
and the right operand is a CSG `union` of small white spheres that cut the pips.
Unlike painted circles, these pips are real recesses; their curved surfaces come
from the sphere cutters, which supply their white material.

## How the rounded cube is constructed

The finished die extends from -1 to +1 on each axis. Its rounding radius is
`r = 0.16`, so the edge and corner centers are at `a = 1 - r = 0.84`.

| Pieces | Geometry |
| --- | --- |
| 3 face boxes | Cube scales `[1, a, a]`, `[a, 1, a]`, and `[a, a, 1]` |
| 12 rounded edges | Radius-r cylinders between -a and +a, centered on each edge and rotated along X, Y, or Z |
| 8 rounded corners | Radius-r spheres centered at all combinations of `[±a, ±a, ±a]` |

Join these pieces with nested binary CSG unions. The union discards internal
surfaces and leaves flat faces, cylindrical edge fillets, and spherical corners.
The YAML anchor `rounded_parts` reuses the same geometry for all three dice.

Each pip cutter has radius `0.18` and sits just outside a face, at coordinate
`±1.045` along its normal. It overlaps the die enough to carve a shallow bowl.
The pips on opposite faces add up to seven.

## Editing CSG in the scene editor

- **+ CSG** creates a union with two default operands.
- Expand its object-tree row to select **Left** or **Right**, then edit the
  operand's shape, transform, source (for OBJ), or material normally.
- **Operation** selects union, intersection, or difference.
- **Wrap in CSG** nests the selected object in a new CSG, allowing more complex
  constructions without editing YAML.
- **Unwrap → Keep Left / Keep Right** removes the selected CSG and discards the
  other operand. It keeps visibility and applied material/shadow overrides.
  If the outer CSG has a non-identity transform, a one-child group retains that
  transform so the chosen operand does not move or become distorted.
- CSG has exactly two operands. Direct operand hide/remove controls are disabled;
  hide or remove the whole CSG instead, or change an operand's shape.
- **Override operand materials** applies the selected CSG's material throughout
  its operands. Leave it off at the die root to keep body and pip colors distinct.
  Select the **rounded body** CSG to edit just the colored shell, or the
  **Pip cutters** CSG to edit just the recessed surfaces.

The YAML form is `type: csg`, an `operation`, and two `children` in left/right
order. An optional `material` overrides the operand materials. Transform,
visibility, and `cast_shadow` work on the complete CSG.
