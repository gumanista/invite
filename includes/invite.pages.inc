<?php

/**
* Invire view callback.
*/
function invite_view($invite) {
  drupal_set_title(entity_label('invite', $invite));
  return entity_view('invite', array(entity_id('invite', $invite) => $invite), 'full');
}

function invite_accept($invite) {
  global $user;

  if (!$user->uid && $invite->status() == INVITE_VALID) {
    $_SESSION[INVITE_SESSION] = $invite->reg_code;
    drupal_goto(variable_get('invite_registration_path', 'user/register'));
  }
  else {
    switch ($invite->status()) {
      case INVITE_WITHDRAWN:
        drupal_set_message(t('This invitation has been withdrawn.'));
        break;

      case INVITE_USED:
        drupal_set_message(t('This invitation has already been used.'));
        break;

      case INVITE_EXPIRED:
        drupal_set_message(t('This invitation has expired.'));
        break;
    }
  }


}