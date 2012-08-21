<?php

/**
* Invire view callback.
*/
function invite_view($invite) {
  dpm(get_class($invite));
  dpm($invite->status());
  drupal_set_title(entity_label('invite', $invite));
  return entity_view('invite', array(entity_id('invite', $invite) => $invite), 'full');
}