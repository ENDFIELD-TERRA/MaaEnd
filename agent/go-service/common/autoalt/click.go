package autoalt

import (
	"encoding/json"

	maa "github.com/MaaXYZ/maa-framework-go/v4"
	"github.com/rs/zerolog/log"

	"github.com/MaaXYZ/MaaEnd/agent/go-service/pkg/pienv"
)

const (
	autoAltClickKeyDownNode = "__AutoAltClickAltKeyDownAction"
	autoAltClickMouseNode   = "__AutoAltClickMouseClickAction"
	autoAltClickKeyUpNode   = "__AutoAltClickAltKeyUpAction"
)

type autoAltClickParam struct {
	// TargetOffset is an optional [dx, dy, dw, dh] offset applied to the
	// recognition box before clicking, matching the semantics of the
	// built-in Click action's target_offset.
	TargetOffset []int `json:"target_offset,omitempty"`
}

type AutoAltClickAction struct{}

// Compile-time interface check
var _ maa.CustomActionRunner = &AutoAltClickAction{}

func (a *AutoAltClickAction) Run(ctx *maa.Context, arg *maa.CustomActionArg) bool {
	if ctx == nil || arg == nil {
		log.Error().Str("component", "AutoAltClickAction").Msg("modifier click action received nil context or arg")
		return false
	}

	box := arg.Box
	// custom_action_param is optional; only parse it when present.
	if param := arg.CustomActionParam; param != "" {
		var p autoAltClickParam
		if err := json.Unmarshal([]byte(param), &p); err != nil {
			log.Warn().
				Err(err).
				Str("component", "AutoAltClickAction").
				Str("custom_action_param", param).
				Msg("failed to parse custom action param, clicking the box without offset")
		} else if len(p.TargetOffset) == 4 {
			box = maa.Rect{
				box[0] + p.TargetOffset[0],
				box[1] + p.TargetOffset[1],
				box[2] + p.TargetOffset[2],
				box[3] + p.TargetOffset[3],
			}
		}
	}

	// CloudWeb: the page does not forward Alt+click to the game, so the
	// shared menu entry would retry forever. Escape reaches the game and is
	// handled by the sidecar through CDP; keep the retry loop around it.
	if pienv.ResolveControllerKind(nil) == pienv.ControllerKindCloudWeb {
		return runPipelineAction(ctx, "AutoAltClickAction", "__AutoEscClickAction", box)
	}

	return runModifierClickActions(
		ctx,
		"AutoAltClickAction",
		autoAltClickKeyDownNode,
		autoAltClickMouseNode,
		autoAltClickKeyUpNode,
		box,
	)
}
