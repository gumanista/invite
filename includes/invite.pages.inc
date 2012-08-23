<?php

/**
* Invte view callback.
*/
function invite_view($invite) {
  drupal_set_title(entity_label('invite', $invite));
  dpm(entity_view('invite', array(entity_id('invite', $invite) => $invite), 'full'));
  return entity_view('invite', array(entity_id('invite', $invite) => $invite), 'full');
}

function invite_accept($invite) {
  global $user;

  $redirect = '<front>';
  $message = '';

  if ($user->uid == $invite->uid) {
    $message = t('You could not use own invite.');
    $redirect = '<front>';
  }
  elseif (!$user->uid && $invite->status() == INVITE_VALID) {
    $_SESSION[INVITE_SESSION_CODE] = $invite->reg_code;
    $redirect = variable_get('invite_registration_path', 'user/register');
  }
  else {
    switch ($invite->status()) {
      case INVITE_WITHDRAWN:
        $message = t('This invitation has been withdrawn.');
        break;

      case INVITE_USED:
        $message = t('This invitation has already been used.');
        break;

      case INVITE_EXPIRED:
        $message = t('This invitation has expired.');
        break;

      default:
        $redirect = 'user';
    }
  }

  if (!empty($message)) {
    drupal_set_message($message);
  }
  drupal_goto($redirect);
}

/**
 * Menu callback; resend an expired invite.
 *
 * @param $invite
 *   An invitate object.
 */
function invite_resend($invite) {
  global $user;

  // Inviter must match current user and invitation must have expired.
  if ($invite->uid == $user->uid && $invite->expiry < REQUEST_TIME && $invite->joined == 0 && $invite->canceled == 0) {
    $invite->sendInvite();
  }
  else {
    return MENU_ACCESS_DENIED;
  }


}

