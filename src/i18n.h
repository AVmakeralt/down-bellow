#ifndef TV_I18N_H
#define TV_I18N_H

/*
 * Localization hook. For now this is English-only and _() is a passthrough,
 * but every user-visible string in the game should be wrapped in _() so we
 * can swap in a gettext-style table later without a find-replace pass.
 *
 * Usage:
 *     const char* line = _("You... who are you?");
 *
 * Future: when we add languages, this becomes a table lookup keyed by
 * the (locale, string) pair, falling back to the original literal.
 */

#define _(s) (s)

#endif /* TV_I18N_H */
