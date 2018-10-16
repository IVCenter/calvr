#include <cvrUtil/glesDrawable.h>
using namespace cvr;

bool glesDrawable::PushAllState() const
{
    glState state;

    state.blend = glIsEnabled(GL_BLEND);
    state.depthTest = glIsEnabled(GL_DEPTH_TEST);
    state.cullFace = glIsEnabled(GL_CULL_FACE);
    state.dither = glIsEnabled(GL_DITHER);
    state.polygonOffsetFill = glIsEnabled(GL_POLYGON_OFFSET_FILL);
    state.scissorTest = glIsEnabled(GL_SCISSOR_TEST);
    state.stencilTest = glIsEnabled(GL_STENCIL_TEST);

    _stateStack->push(state);
    return true;
}
bool glesDrawable::PopAllState() const
{
    if (!_stateStack->empty()) {
        glState state = _stateStack->top();

        if (state.blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (state.depthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (state.cullFace) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (state.dither) glEnable(GL_DITHER); else glDisable(GL_DITHER);
        if (state.polygonOffsetFill) glEnable(GL_POLYGON_OFFSET_FILL); else glDisable(GL_POLYGON_OFFSET_FILL);
        if (state.scissorTest) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        if (state.stencilTest) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);

        _stateStack->pop();
    }
    return true;
}